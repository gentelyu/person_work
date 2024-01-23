#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "StdTcp.h"
#include "StdFile.h"
#include "StdSqlite.h"
#include "StdThread.h"
#include "DoubleLinkList.h"
#include "ThreadPool.h"
#include <time.h>

#include <unistd.h>
#define true 1
#define false 0
// typedef struct CallbackParameter//回调函数参数
// {
//     sqlite3 *db;
//     int acceptSork;
// } CP;

typedef struct Message // 数据包
{
    int flag;
    int back;
    char sock[30];

    char fromName[20];
    char toName[20];
    char content[1024]; // 消息
    char order[2048];   // 让服务器执行的指令
    char order1[4096];


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

void *thread_handler(void *arg1, void *arg2)
{
    sqlite3 *sq2 = GetSqlDb((SQL *)arg2);

    time_t currentTime; // 时间函数
    struct tm *localTime;

    int sock = (*(int *)arg1);
    while (1)
    {
        fflush(stdin);

        Msg msg = {0, 0, {0}, {0}, {0}, {0}, {0}, {0}};

        TcpServerRecv(sock, &msg, sizeof(msg));
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
        // case 1:
        // DBInsertTail(&list,CreateClient(sock,msg.fromName));
        //     break;
        // case 2://查找名字匹配msg.toName的客户端对象，并向其发送消息。
        //     struct Node* travelPoint = list.head;
        //     while (travelPoint != NULL)
        //     {
        //         C* c = (C *)travelPoint->data;
        //         if(strcmp(c->name,msg.toName) == 0)
        //         {
        //             TcpServerSend(c->sock,&msg,sizeof(msg));
        //             break;
        //         }
        //         travelPoint = travelPoint->next;
        //     }
        //     break;
        // case 3://遍历整个链表向除了自己以外的所有人小消息
        //     struct Node* travelPoint1 = list.head;
        //     while (travelPoint1 != NULL)
        //     {
        //         C* c = (C *)travelPoint1->data;
        //         if(strcmp(c->name,msg.fromName) != 0)
        //         {
        //             TcpServerSend(c->sock,&msg,sizeof(msg));
        //         }
        //         travelPoint1 = travelPoint1->next;
        //     }
        //     break;
        case 1:
            // 用户注册
            
            if (sqlite3_exec(sq2, msg.order, NULL, NULL, NULL) != SQLITE_OK) // 执行不成功，说明已有账户
            {
                printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
                msg.flag = 1;
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            else
            {
                msg.flag = 2;
                
                // 建立好友表
                char *prolist[] = {"account", "TEXT PRIMARY KEY NOT NULL", "name", "TEXT NOT NULL"};

                CreateTable((SQL *)arg2, msg.fromName, prolist, sizeof(prolist) / sizeof(prolist[0]) / 2);

                //建立我的群聊列表
                sprintf(msg.content,"%sGroup",msg.fromName);
                char *prolist2[] = {"Groupname", "TEXT NOT NULL"};

                CreateTable((SQL *)arg2, msg.content, prolist2, sizeof(prolist2) / sizeof(prolist2[0]) / 2);

                // 建立通知表
                char informlist[50] = {0};
                char *prolist1[] = {"name", "TEXT NOT NULL", "message", "TEXT NOT NULL", "state", "TEXT NOT NULL", "time", "int"};
                sprintf(informlist, "%sinformlist", msg.fromName);
                CreateTable((SQL *)arg2, informlist, prolist1, sizeof(prolist1) / sizeof(prolist1[0]) / 2);

                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
                
            }
            break;
        case 2: // 登录
            msg.flag = 3;
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
                UpdateData((SQL *)arg2, "user", "state = '在线'", where);// 改为在线状态
                UpdateData((SQL *)arg2, "user", msg.sock, where); // 每次登陆换上新套接字
                memset(msg.fromName, 0, sizeof(msg.fromName));
            }
            else
            {
                msg.flag = 8;
                strcpy(msg.content, "用户名或密码错误，登录失败!\n");
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            break;
        case 3: // 互发消息
            
            msg.flag = 100;
            // printf("%d",msg.flag);
            sprintf(msg.order, "select sock,state from user where account = '%s';", msg.toName);
            SelectInfo((SQL *)arg2, msg.order, &result, &row, &colunm);

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
        case 4: // 发送加好友
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
                msg.flag = 4;
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            else
            {
                msg.flag = 5;
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            memset(msg.content, 0, sizeof(msg.content));
            memset(msg.toName, 0, sizeof(msg.toName));
            break;
        case 5: // 同意加好友
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
                msg.flag = 6;
                printf("SelectInfo error:%s", sqlite3_errmsg(sq2));
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
                return false;
            }
            else
            {
                msg.flag = 7;
                strcpy(msg.fromName,"傻妞");
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            memset(msg.order, 0, sizeof(msg.order));
            memset(msg.order1, 0, sizeof(msg.order));
            break;
        case 6://查看所有消息
            FreeInfoReault(result);

            sprintf(msg.order, "%sinformlist", msg.fromName);
            GetTableInfo((SQL *)arg2, msg.order, &result, &row, &colunm);
            for (int i = 0; i < row; i++)
            {
                msg.flag = 3;
                printf("%d\n",msg.flag);
                sprintf(msg.content, "[%s]..[%s]", result[5 + 4 * i], result[6 + 4 * i]);
                

                TcpServerSend(sock, &msg, sizeof(msg));
            }
            
            sprintf(msg.order1,"select *from '%sinformlist' where state = '未读';",msg.fromName);
            SelectInfo((SQL*)arg2,msg.order1,&result,&row,&colunm);
            if(row > 0)
            {
                char where[100] = {0};
                sprintf(where,"state = '未读'");
                UpdateData((SQL *)arg2,msg.order, "state = '已读'", where);
                memset(msg.content, 0, sizeof(msg.content));
                memset(where, 0, sizeof(where));
            }

            memset(msg.order, 0, sizeof(msg.order));
            memset(msg.order1, 0, sizeof(msg.order1));
            FreeInfoReault(result);
            break;
        case 7://查看单条消息
            FreeInfoReault(result);
            msg.flag = 8;
            sprintf(msg.order,"select message from '%sinformlist' where state = '未读' order by time asc limit 1;",msg.fromName);
            SelectInfo((SQL*)arg2,msg.order,&result,&row,&colunm);
            if(row == 0)
            {
                msg.flag = 9;
                TcpServerSend(sock,&msg,sizeof(msg));
                break;
            }
            strcpy(msg.content,result[1]);
            
            TcpServerSend(sock,&msg,sizeof(msg));
            char *tableName = {0};

            sprintf(msg.order, "%sinformlist", msg.fromName);
            UpdateData((SQL*)arg2,msg.order,"state = '已读'","state = '未读'  order by time asc limit 1");
            memset(msg.order, 0, sizeof(msg.order));
            break;
        case 9:
            msg.flag = 100;
            //sprintf(msg.content,"%sGroup",msg.content);

            char *prolist4[] = {"account", "TEXT PRIMARY KEY NOT NULL", "name", "TEXT NOT NULL"};
            CreateTable((SQL*)arg2,msg.content,prolist4,sizeof(prolist4) / sizeof(prolist4[0]) / 2);
          
            sprintf(msg.order,"select name from user where account = '%s'",msg.fromName);
            SelectInfo((SQL*)arg2,msg.order,&result,&row,&colunm);
            
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
        case 10://拉人
           
            //sprintf(msg.content,"%sGroup",msg.fromName);
            sprintf(msg.order,"select name from user where account = '%s'",msg.toName);
            SelectInfo((SQL*)arg2,msg.order,&result,&row,&colunm);
            sprintf(msg.order,"insert into %s values('%s','%s');",msg.content,msg.toName,result[1]);
            sprintf(msg.order1,"insert into %sGroup values (%s)",msg.toName,msg.content);
            a = sqlite3_exec(sq2, msg.order, NULL, NULL, NULL); 
            b = sqlite3_exec(sq2, msg.order1, NULL, NULL, NULL); 
            if (a != SQLITE_OK && b != SQLITE_OK) // 
            {
                msg.flag  == 11;
                printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
    
                strcpy(msg.fromName,"傻妞");
                
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            // else
            // {
            //     msg.flag = 10;
            //     strcpy(msg.fromName,"傻妞");
                
            //     TcpServerSend(sock, &msg, sizeof(msg));
            // }

            memset(msg.order, 0, sizeof(msg.order));
            //更新对方群聊列表
            sprintf(msg.order,"insert into %sGroup values('%s');",msg.toName,msg.content);
            if (sqlite3_exec(sq2, msg.order, NULL, NULL, NULL) != SQLITE_OK) // 
            {
                msg.flag = 11;
                printf("insert table error :%s!\n", sqlite3_errmsg(sq2));
    
                strcpy(msg.fromName,"傻妞");
               
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            else
            {
                msg.flag = 10;
                strcpy(msg.fromName,"傻妞");
                
                TcpServerSend(sock, &msg, sizeof(msg));
            }
            break;
        case 11:
                msg.flag = 12;
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

                // sprintf(msg.order,"select sock from user where account != '%s'and attribute = 'sock';",msg.fromName);
                // sqlite3_get_table(sq2,msg.order,&result,&row,&colunm,NULL);
                // for(int i = 0;i < row;i++)
                // {
                //     BigSock = atoi(result[i + 1]);
                //     TcpServerSend(BigSock,&msg,sizeof(msg));
                // }

            break;
        case 12:
                
                msg.flag = 100;
                currentTime = time(NULL);
                localTime = localtime(&currentTime);
                sprintf(msg.order, "select sock,state from user where account = '%s';", msg.toName);
                
                SelectInfo((SQL *)arg2, msg.order, &result, &row, &colunm);
                
                BigSock = atoi(result[2]);
                
                if (strcmp(result[3], "在线") == 0)
                {
                    
                    sprintf(msg.order,"%s 向你发送了一个[%s]的文件",msg.fromName,msg.content);
                    
                    printf("%s,%s,%s\n",msg.toName,msg.fromName,msg.order);
                    sprintf(msg.order1, "insert into %sinformlist values('%s','%s','未读',%d%d%d%d%d%d);", msg.toName, msg.fromName,\
                   msg.order, localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,localTime->tm_hour,localTime->tm_min,localTime->tm_sec);
                    printf("oooooo---oooo\n");
                    printf("-----------%s-----------------\n",msg.order1);
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
        case 13:
                char temp[100] = {0};
                strcpy(temp,LoadFromFile(msg.content));
                printf("%s\n",temp);
                msg.back = strlen(temp);
                strcpy(msg.content,temp);
                msg.flag = 13;
                TcpServerSend(sock, &msg, sizeof(msg));
            break;
        // case 15:
                
        //         sprintf(msg.order,"select account from %s where account != '%s';",msg.fromName,msg.fromName);
        //         SelectInfo((SQL*)arg2,msg.order,&result,&row,&colunm);
        //         if(row == 0)
        //         {
        //             msg.flag = 0;
        //             strcpy(msg.content,"您暂时还没有好友");
        //             TcpServerSend(sock,&msg,sizeof(msg));
        //             break;
        //         }
        //         else if(row > 0)
        //         {
        //             for(int i = 0; i < row ; i++)
        //             {
        //                 sprintf(msg.order1,"select state from user where account = '%s';",result[1 + i]);
        //                 SelectInfo((SQL*)arg2,msg.order1,&result1,&row1,&colunm1);
        //                 if(result1[1] == "在线")
        //                 {
        //                     msg.flag = 100;
        //                     strcpy(msg.fromName,"傻妞");
        //                     strcpy(msg.content,result1[1]);
        //                     TcpServerSend(sock,&msg,sizeof(msg));
        //                 }
                        
        //             }
                    
        //         }
                
                

        //         break;
        default:
            break;
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3) // 接受三个参数,这三个参数就是在终端输入的三个参数
    {
        printf("invalid nums!\n");
        return -1;
    }

    //InitDLlist(&list);

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

    SQL *sq = InitSqlite("Users.db"); // 初始化数据库
    // sqlite3 *sq2 = GetSqlDb(sq);

    int acceptSork = 0;
    while ((acceptSork = TcpServerAccept(s)) > 0) // 每个初始化建立连接后的新套接字不变
    {
        Threadp_AddTask(p, thread_handler, &acceptSork, sq);
    }

    DestoryThreadPool(p);

    return 0;
}
