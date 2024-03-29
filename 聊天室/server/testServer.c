#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sqlite3.h>

#include "DoubleLinkList.h"
#include "StdFile.h"
#include "StdSqlite.h"
#include "StdTcp_Server.h"
#include "StdThread.h"
#include "ThreadPool.h"


#define true 1
#define false 0
#if 1
const char * server_ip = "172.16.136.101";
#else
    /* 本地环回地址 */
    const char *server_ip = "127.0.0.1";
#endif

#define server_port 8080

#define SOCK_SIZE 30
#define FROMENAME_SIZE 20
#define TONAME_SIZE 20
#define CONTENT_SIZE 1024
#define ORDER_SIZE 2048
#define ORDER1_SIZE 4096

/* 状态码 */
enum status_client_to_server
{
    LOGIN,
    REGISTRE,
    SEND_MESSAGE,   //发消息
    ADD_FRIENDS = 4,            //添加好友
    AGREE_FRIENDS_APPLY= 5,     //同意好友请求
    READ_ALL_INFO = 6,          //查看所有通知
    READ_GINGLE_INFO = 7,       //按条查看通知
    CREATE_GROUP = 9,            //创建群聊
    GROUP_ADD = 10,              //群聊拉人
    GROUP_SEND_MESSAGE = 11,         //发送群消息
    SEND_FILE = 12,              //发送文件
    RECV_FILE = 13              //接收文件 
};

enum status_server_to_client
{
    S_TO_C_ADD_USER_ERROR = 700,     //创建用户失败  1
    S_TO_C_ADD_USER_SUCCESS = 701,   //创建用户成功  
    S_TO_C_READ_INFORMATION = 702,   //接收通知
    S_TO_C_SEND_ADD_FRIEND_ERROR = 703,  //发送添加好友失败
    S_TO_C_SEND_ADD_FRIEND_SUCCESS = 704,    //发送添加好友成功
    S_TO_C_UPDATE_FRIENDSHIP_ERROR = 705,    //更新好友列表失败
    S_TO_C_UPDATE_FRIENDSHIP_SUCCESS = 706,    //更新好友列表成功
    S_TO_C_LOGIN_ERROR = 707,                    //登陆失败
    S_TO_C_INFORMATION_EMPTY = 708,              //当前已无未读消息
    S_TO_C_UPDATE_GROUP_SUCCESS = 709,           //群聊列表更新成功
    S_TO_C_UPDATE_GROUP_ERROR = 710,             //群聊列表根棍失败
    S_TO_C_SEND_GROUP_MESSAGE = 711,             //发送群消息
    S_TO_C_SEND_MESSAGE = 712,                //发消息
};


typedef struct Message // 数据包
{
    int flag;
    int back;
    char sock[SOCK_SIZE];

    char fromName[FROMENAME_SIZE];
    char toName[TONAME_SIZE];
    char content[CONTENT_SIZE]; // 消息
    char order[ORDER_SIZE];   // 让服务器执行的指令
    char order1[ORDER1_SIZE];


} Msg;

typedef struct Client // 定义一个名字，通过名字找到套接字
{
    int sock;
    char name[20];
} C;

C *CreateClient(int sock, const char *name) // 创建一个客户端，有名字有套接字
{
    C *c = (C *)malloc(sizeof(C));
    if (c == NULL)
    {
        printf("CreateClient malloc error!\n");
        return NULL;
    }
    c->sock = sock;
    strcpy(c->name, name);
    return c;
}

void FreeClient(C *c) // 释放客户端结构体
{
    if (c != NULL)
    {
        free(c);
    }
}

DLlist list;

void *thread_handler(void *arg2)
{
    printf("thread_handler\n");
    TcpS * arg = (TcpS *)arg2;
    sqlite3 *sq2 = GetSqlDb(arg->sql);

    time_t currentTime; // 时间函数
    struct tm *localTime;

    int sock = arg->communicate_sock;
    printf("thread_handler\n");
    while (1)
    {
        fflush(stdin);

        Msg msg = {0, 0, {0}, {0}, {0}, {0}, {0}, {0}};
        int serverRecv = 0;
        printf("TcpServerRecv\n");
        serverRecv = TcpServerRecv(arg, sock, &msg, sizeof(msg));
        if (serverRecv <= 0)
        {
            break;
        }
        
        printf("recv flag : %d,fromname : %s,toname : %s,content: %s,order :%s\n",
               msg.flag, msg.fromName, msg.toName, msg.content, msg.order);

        // char ***result, int *row, int *column
        char **result = NULL; // 存放执行语句的结果
        char **result1 = NULL;
        int row;          // 查找到的行数
        int colunm;       // 查找打的列数
        int row1;
        int colunm1;

        switch (msg.flag)
        {
        case REGISTRE:
            // 用户注册
            
            if (sqlite3_exec(sq2, msg.order, NULL, NULL, NULL) != SQLITE_OK) // 执行不成功，说明已有账户
            {
                printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
                msg.flag = S_TO_C_ADD_USER_ERROR;
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            else
            {
                msg.flag = S_TO_C_ADD_USER_SUCCESS;
                
                // 建立好友表
                char *prolist[] = {"account", "TEXT PRIMARY KEY NOT NULL", "name", "TEXT NOT NULL"};

                CreateTable(arg->sql, msg.fromName, prolist, sizeof(prolist) / sizeof(prolist[0]) / 2);

                //建立我的群聊列表
                sprintf(msg.content,"%sGroup",msg.fromName);
                char *prolist2[] = {"Groupname", "TEXT NOT NULL"};

                CreateTable(arg->sql, msg.content, prolist2, sizeof(prolist2) / sizeof(prolist2[0]) / 2);

                // 建立通知表
                char informlist[50] = {0};
                char *prolist1[] = {"name", "TEXT NOT NULL", "message", "TEXT NOT NULL", "state", "TEXT NOT NULL", "time", "int"};
                sprintf(informlist, "%sinformlist", msg.fromName);
                CreateTable(arg->sql, informlist, prolist1, sizeof(prolist1) / sizeof(prolist1[0]) / 2);

                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
                
            }
            break;
        case LOGIN: // 登录
            msg.flag = S_TO_C_READ_INFORMATION;
            if (sqlite3_get_table(sq2, msg.order, &result, &row, &colunm, NULL) != SQLITE_OK)
            {
                printf("SelectInfo error:%s", sqlite3_errmsg(sq2));
                return false;
            }

            if (result[1] != NULL && row > 0)
            {
                
                sprintf(msg.content, "登陆成功，%s!\n", result[1]);
                char where[100] = {0};
                sprintf(where, "account = '%s'", msg.fromName);
                
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg)); 

                
                sprintf(msg.sock, "sock = %d", sock);
                UpdateData(arg->sql, "user", "state = '在线'", where);// 改为在线状态
                UpdateData(arg->sql, "user", msg.sock, where); // 每次登陆换上新套接字
                memset(msg.fromName, 0, sizeof(msg.fromName));
            }
            else
            {
                msg.flag = S_TO_C_LOGIN_ERROR;
                strcpy(msg.content, "用户名或密码错误，登录失败!\n");
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            break;
        case SEND_MESSAGE: // 互发消息
            
            msg.flag = S_TO_C_SEND_MESSAGE;
            // printf("%d",msg.flag);
            sprintf(msg.order, "select sock,state from user where account = '%s';", msg.toName);
            SelectInfo(arg->sql, msg.order, &result, &row, &colunm);

            int BigSock = atoi(result[2]);
            if (strcmp(result[3], "在线") == 0)
            {
                TcpServerSend(BigSock, &msg, sizeof(msg));
            }
            else
            {
                strcpy(msg.content, "好友已下线");
                TcpServerSend(sock, &msg, sizeof(msg));
                // 把消息存到数据库或者文件里，下次好友上线，直接发给他。
            }
            result = 0;
            break;
        case ADD_FRIENDS: // 发送加好友
            currentTime = time(NULL);
            localTime = localtime(&currentTime); // 获取当前时间

            memset(msg.content, 0, sizeof(msg.content));
            sprintf(msg.content, "%s 向您发出好友申请!", msg.fromName);
            memset(msg.order, 0, sizeof(msg.order));
            sprintf(msg.order, "insert into %sinformlist values('%s','%s','未读',%d%d%d%d%d%d);", msg.toName, msg.fromName,\
                   msg.content, localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,localTime->tm_hour,localTime->tm_min,localTime->tm_sec);

            //printf("%s\n", msg.order);

            if (sqlite3_exec(sq2, msg.order, NULL, NULL, NULL) != SQLITE_OK) // 执行不成功
            {
                printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
                msg.flag = S_TO_C_SEND_ADD_FRIEND_ERROR;
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            else
            {
                msg.flag = S_TO_C_SEND_ADD_FRIEND_SUCCESS;
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            memset(msg.content, 0, sizeof(msg.content));
            memset(msg.toName, 0, sizeof(msg.toName));
            break;
        case AGREE_FRIENDS_APPLY: // 同意加好友
            memset(msg.order, 0, sizeof(msg.order));
            memset(msg.order1, 0, sizeof(msg.order));
            
            sprintf(msg.order, "select name from user where account = '%s';", msg.toName); // 找到对方昵称
            sqlite3_get_table(sq2, msg.order, &result, &row, &colunm, NULL);

            memset(msg.order, 0, sizeof(msg.order));
            sprintf(msg.order, "insert into %s values('%s','%s');", msg.fromName, msg.toName, result[1]); // 更新自己好友表

            FreeInfoReault(result);                                                           // 清空垃圾值
            sprintf(msg.order1, "select name from user where account = '%s';", msg.fromName); // 找到自己的昵称
            sqlite3_get_table(sq2, msg.order1, &result, &row, &colunm, NULL);
            memset(msg.order1, 0, sizeof(msg.order));

            sprintf(msg.order1, "insert into %s values('%s','%s');", msg.toName, msg.fromName, result[1]); // 更新对方好友列表

            int a = sqlite3_exec(sq2, msg.order, NULL, NULL, NULL);
            int b = sqlite3_exec(sq2, msg.order1, NULL, NULL, NULL);
            if (a != SQLITE_OK && b != SQLITE_OK)
            {
                msg.flag = S_TO_C_UPDATE_FRIENDSHIP_ERROR;
                printf("SelectInfo error:%s", sqlite3_errmsg(sq2));
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
                return false;
            }
            else
            {
                msg.flag = S_TO_C_UPDATE_FRIENDSHIP_SUCCESS;
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            memset(msg.order, 0, sizeof(msg.order));
            memset(msg.order1, 0, sizeof(msg.order));
            break;
        case READ_ALL_INFO://查看所有消息
            FreeInfoReault(result);

            sprintf(msg.order, "%sinformlist", msg.fromName);
            GetTableInfo(arg->sql, msg.order, &result, &row, &colunm);
            if(row == 0)
            {
                msg.flag = S_TO_C_INFORMATION_EMPTY;
                TcpServerSend(sock,&msg,sizeof(msg));
                break;
            }
            for (int i = 0; i < row; i++)
            {
                msg.flag = S_TO_C_READ_INFORMATION;
                printf("%d\n",msg.flag);
                sprintf(msg.content, "[%s]..[%s]", result[5 + 4 * i], result[6 + 4 * i]);
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            
            sprintf(msg.order1,"select *from '%sinformlist' where state = '未读';",msg.fromName);
            SelectInfo(arg->sql,msg.order1,&result,&row,&colunm);
            if(row > 0)
            {
                char where[100] = {0};
                sprintf(where,"state = '未读'");
                UpdateData(arg->sql,msg.order, "state = '已读'", where);
                memset(msg.content, 0, sizeof(msg.content));
                memset(where, 0, sizeof(where));
            }

            memset(msg.order, 0, sizeof(msg.order));
            memset(msg.order1, 0, sizeof(msg.order1));
            FreeInfoReault(result);
            break;
        case READ_GINGLE_INFO://查看单条消息
            FreeInfoReault(result);
            msg.flag = S_TO_C_READ_INFORMATION;
            sprintf(msg.order,"select message from '%sinformlist' where state = '未读' order by time asc limit 1;",msg.fromName);
            SelectInfo(arg->sql,msg.order,&result,&row,&colunm);
            if(row == 0)
            {
                msg.flag = S_TO_C_INFORMATION_EMPTY;
                TcpServerSend(sock,&msg,sizeof(msg));
                break;
            }
            strcpy(msg.content,result[1]);
            
            TcpServerSend(sock,&msg,sizeof(msg));
            char *tableName = {0};

            sprintf(msg.order, "%sinformlist", msg.fromName);
            UpdateData(arg->sql,msg.order,"state = '已读'","state = '未读'  order by time asc limit 1");
            memset(msg.order, 0, sizeof(msg.order));
            break;
        case CREATE_GROUP:
            msg.flag = S_TO_C_READ_INFORMATION;
            //sprintf(msg.content,"%sGroup",msg.content);

            char *prolist4[] = {"account", "TEXT PRIMARY KEY NOT NULL", "name", "TEXT NOT NULL"};
            CreateTable(arg->sql,msg.content,prolist4,sizeof(prolist4) / sizeof(prolist4[0]) / 2);
          
            sprintf(msg.order,"select name from user where account = '%s'",msg.fromName);
            SelectInfo(arg->sql,msg.order,&result,&row,&colunm);
            
            sprintf(msg.order,"insert into %s values('%s','%s');",msg.content,msg.fromName,result[1]);
            sprintf(msg.order1,"insert into %sGroup values('%s')",msg.fromName,msg.content);
            a = sqlite3_exec(sq2, msg.order, NULL, NULL, NULL);
            b = sqlite3_exec(sq2, msg.order1, NULL, NULL, NULL);
            if (a != SQLITE_OK && b != SQLITE_OK) // 执行不成功，说明已有账户
            {
                printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
    
                strcpy(msg.fromName,"傻妞");
                strcpy(msg.content,"群聊创建失败");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            else
            {
                strcpy(msg.fromName,"傻妞");
                strcpy(msg.content,"群聊创建成功");
                TcpServerSend(sock, &msg, sizeof(msg));
            }

            
            break;
        case GROUP_ADD://拉人
           
            //sprintf(msg.content,"%sGroup",msg.fromName);
            sprintf(msg.order,"select name from user where account = '%s'",msg.toName);
            SelectInfo(arg->sql,msg.order,&result,&row,&colunm);
            sprintf(msg.order,"insert into %s values('%s','%s');",msg.content,msg.toName,result[1]);
            sprintf(msg.order1,"insert into %sGroup values (%s)",msg.toName,msg.content);
            a = sqlite3_exec(sq2, msg.order, NULL, NULL, NULL); 
            b = sqlite3_exec(sq2, msg.order1, NULL, NULL, NULL); 
            if (a != SQLITE_OK && b != SQLITE_OK) // 
            {
                msg.flag  == S_TO_C_UPDATE_GROUP_ERROR;
                printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
    
                strcpy(msg.fromName,"傻妞");
                
                TcpServerSend(sock, &msg, sizeof(msg));
            }

            memset(msg.order, 0, sizeof(msg.order));
            //更新对方群聊列表
            sprintf(msg.order,"insert into %sGroup values('%s');",msg.toName,msg.content);
            if (sqlite3_exec(sq2, msg.order, NULL, NULL, NULL) != SQLITE_OK) // 
            {
                msg.flag = S_TO_C_UPDATE_GROUP_ERROR;
                printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
    
                strcpy(msg.fromName,"傻妞");
               
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            else
            {
                msg.flag = S_TO_C_UPDATE_GROUP_SUCCESS;
                strcpy(msg.fromName,"傻妞");
                
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            break;
        case GROUP_SEND_MESSAGE:
                msg.flag = S_TO_C_SEND_GROUP_MESSAGE;
                //char **result1[] = {0};

                sprintf(msg.order,"select account from %s where account != '%s';",msg.toName,msg.fromName);
                sqlite3_get_table(sq2,msg.order,&result,&row,&colunm,NULL);
                printf("row: %d  coliunm : %d\n",row,colunm);
                printf("%s    %s\n",result[1],result[2]);
                for(int i = 0; i < row;i++)
                {
                    
                    sprintf(msg.order1,"select sock from user where account = '%s';",result[1 + i]);

                    printf("%s\n",result[1 + i]);

                    sqlite3_get_table(sq2,msg.order1,&result1,&row1,&colunm1,NULL);
                    BigSock = atoi(result1[1]);
                    printf("%d\n",BigSock);

                    TcpServerSend(BigSock,&msg,sizeof(msg));
                }
            break;
        case SEND_FILE:
                
                msg.flag = S_TO_C_READ_INFORMATION;
                currentTime = time(NULL);
                localTime = localtime(&currentTime);
                sprintf(msg.order, "select sock,state from user where account = '%s';", msg.toName);
                
                SelectInfo((SQL *)arg, msg.order, &result, &row, &colunm);
                
                BigSock = atoi(result[2]);
                
                if (strcmp(result[3], "在线") == 0)
                {
                    
                    sprintf(msg.order,"%s 向你发送了一个[%s]的文件",msg.fromName,msg.content);
                    
                    printf("%s,%s,%s\n",msg.toName,msg.fromName,msg.order);
                    sprintf(msg.order1, "insert into %sinformlist values('%s','%s','未读',%d%d%d%d%d%d);", msg.toName, msg.fromName,\
                    msg.order, localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
                    if (sqlite3_exec(sq2, msg.order1, NULL, NULL, NULL) != SQLITE_OK) 
                    {
                        printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
                        strcpy(msg.fromName,"傻妞");
                        strcpy(msg.content,"发送失败");
                        TcpServerSend(sock, &msg, sizeof(msg));
                    }
                    else
                    {
                        strcpy(msg.fromName,"傻妞");
                        strcpy(msg.content,"发送成功");
                        TcpServerSend(sock, &msg, sizeof(msg));
                    }
                    strcpy(msg.fromName,"傻妞");
                    strcpy(msg.content,"您收到一条消息通知");

                    TcpServerSend(BigSock, &msg, sizeof(msg));
                }
                else
                {
                    strcpy(msg.content, "好友已下线");
                    strcpy(msg.fromName,"傻妞");
                    strcpy(msg.content,"发送失败");
                    TcpServerSend(sock, &msg, sizeof(msg));
                    // 把消息存到数据库或者文件里，下次好友上线，直接发给他。
                }
            break;
        case RECV_FILE:
                char temp[100] = {0};
                strcpy(temp,LoadFromFile(msg.content));
                printf("%s\n",temp);
                msg.back = strlen(temp);
                strcpy(msg.content,temp);
                msg.flag = S_TO_C_SEND_MESSAGE;
                TcpServerSend(sock, &msg, sizeof(msg));
            break;
        default:
            break;
        }
    }
}

int main(int argc, char const *argv[])
{
#if 0
    if (argc != 3) // 接受三个参数,这三个参数就是在终端输入的三个参数
    {
        printf("invalid nums!\n");
        return -1;
    }

    ThreadP *p = InitThreadPool(10, 20, 10); // 初始化线程池
    if (p == NULL)
    {
        printf("threadpool init error!\n");
        return -1;
    }

    TcpS *s = InitTcpServer(argv[1], atoi(argv[2])); // 初始化服务器
    if (s == NULL)
    {
        printf("InitTcpServer error!\n");
        return -1;
    }
#else
    ThreadP *p = InitThreadPool(10, 20, 10); // 初始化线程池
    if (p == NULL)
    {
        printf("threadpool init error!\n");
        return -1;
    }

    TcpS *s = InitTcpServer(server_ip, server_port); // 初始化服务器
    if (s == NULL)
    {
        printf("InitTcpServer error!\n");
        return -1;
    }

#endif

    SQL *sq = InitSqlite("Users.db"); // 初始化数据库

#if 1
    /* 将数据库结构体对象以及线程池对象放到Tcp服务器的结构体参数中 */
    s->sql = sq;
    s->pThreadPool = p;
#else
    /* 将要用到的参数放进结构体中 */
    arg *pArg = (arg *)malloc(sizeof(arg));
    if (!pArg)
    {
        printf("malloc error");
        return -1;
    }
    memset(pArg, 0, sizeof(pArg));
    pArg->arg_tcp_server = s;
    pArg->arg_sql = sq;
#endif

    //创建原始用户表
    char *prolist[] = {"account","TEXT NOT NULL PRIMARY KEY","name","TEXT NOT NULL","password","TEXY NOT NULL","state","TEXT NOT NULL","sock","INTEGER"};
    CreateTable(sq, "user", prolist, sizeof(prolist) / sizeof(prolist[0]) / 2);


    // sqlite3 *sq2 = GetSqlDb(sq);

    
#if 0
    int acceptSork = 0;
    while ((acceptSork = TcpServerAccept(s)) > 0) // 每个初始化建立连接后的新套接字不变
    {
        Threadp_AddTask(p, thread_handler, &acceptSork, sq);
    }
#else
//这里将TcpServerAccept函数的返回值改为监听到的数量，
    printf("TcpServerAccept\n");
    TcpServerAccept(s, thread_handler, (void *)s);
    printf("TcpServerAccept\n");





#endif

    DestoryThreadPool(p);

    return 0;
}
