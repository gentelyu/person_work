#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "StdTcp_Client.h"
#include "StdSqlite.h"
#include "StdThread.h"
#include "DoubleLinkList.h"
#include "ThreadPool.h"
#include "StdFile.h"
#include <sqlite3.h>
#include <unistd.h>
#define true 1
#define false 0

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
    S_TO_C_UPDATE_GROUP_ERROR = 710,             //群聊列表更新失败
    S_TO_C_SEND_GROUP_MESSAGE = 711,             //发送群消息
    S_TO_C_SEND_MESSAGE = 712,                //发消息
};

typedef struct Message
{
    int flag;           // 消息类型
    int back;
    char sock[30];
    char fromName[20];  // 发送方
    char toName[20];    // 接收方
    char content[1024]; // 内容
    char order[2048];   // 让服务器执行的指令
    char order1[4096];
} Msg;


Msg msg1;
void *thread_handler(void *arg) // 客户端接收消息
{
    TcpC *c = (TcpC *)arg;
    while (1)
    {
        TcpClientRecv(c, &msg1, sizeof(msg1));
        if(msg1.flag == S_TO_C_ADD_USER_ERROR)//创建用户失败
        {
            printf("账户已存在!\n");
        }
        else if(msg1.flag == S_TO_C_ADD_USER_SUCCESS)
        {
            printf("创建成功!\n");
        }
        else if(msg1.flag == S_TO_C_READ_INFORMATION)
        {
            printf("%s\n",msg1.content);
        }
        else if(msg1.flag == S_TO_C_SEND_ADD_FRIEND_ERROR)
        {
            printf("发送失败!\n");
        }
        else if(msg1.flag == S_TO_C_SEND_ADD_FRIEND_SUCCESS)
        {
            printf("发送成功!\n");
        }
        else if(msg1.flag == S_TO_C_UPDATE_FRIENDSHIP_ERROR)
        {
            printf("添加失败!\n");
        }
        else if(msg1.flag == S_TO_C_UPDATE_FRIENDSHIP_SUCCESS)
        {
            printf("添加成功!\n");
        }
        else if(msg1.flag == S_TO_C_LOGIN_ERROR)
        {
            printf("%s\n",msg1.content);
        }
        else if(msg1.flag == S_TO_C_INFORMATION_EMPTY)
        {
            printf("当前已无未读消息!\n");
        }
        else if(msg1.flag == S_TO_C_UPDATE_GROUP_SUCCESS)
        {
            printf("拉取成功\n");
        }
        else if(msg1.flag == S_TO_C_UPDATE_GROUP_ERROR)
        {
            printf("拉取失败\n");
        }
        else if(msg1.flag == S_TO_C_SEND_GROUP_MESSAGE)
        {
            printf("来自群聊[%s][%s] : %s\n",msg1.toName,msg1.fromName,msg1.content);
        }
        else if(msg1.flag == S_TO_C_SEND_MESSAGE)
        {
            printf("recv from [%s] message : %s\n", msg1.fromName, msg1.content);
        }

    }
}


//DLlist list;



int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("invalid nums!\n");
        return -1;
    }

    TcpC *c = InitTcpClient(argv[1], atoi(argv[2])); // 初始化客户端，并与服务器连接
    if (c == NULL)
    {
        printf("InitTcpClient error!\n");
        return -1;
    }

    Msg msg = {0,0,{0},{0},{0},{0},{0},{0}};
    
    Thread *t = InitThread(thread_handler, c); // 初始化线程

    //****************************************************************
    char *num = (char *)malloc(sizeof(char));
    char *acount = (char *)malloc(sizeof(char) * 20);
    char *name = (char *)malloc(sizeof(char) * 20);
    char *password = (char *)malloc(sizeof(char) * 20);
    char *tableName = "user";
    // SQL *s = InitSqlite("Users.db");
    //InitDLlist(&list);
    printf("=========欢迎来到妲己的游戏工坊，我是你的游戏助手妲己，准备开启这场奇妙之旅吧！========\n");
    printf("=====================当前已有账户,请输入2,若没有账户请输入1======================\n");
    scanf("%s", num);

    if(strcmp(num,"1") != 0 && strcmp(num,"2") != 0)
    {
        printf("请检查您的输入!\n");
    }

    while (strcmp(num, "1") == 0)
    {
        msg.flag = REGISTRE;

        printf("=========妲己来帮你注册啦=========\n");
        printf("=========请输入你的ID哦=========\n");
        scanf("%s", acount);
        while (getchar() != '\n');
        printf("=========请输入你的昵称哦=========\n");
        scanf("%s", name);
        while (getchar() != '\n');
        printf("=========请输入你的密码哦=========\n");
        scanf("%s", password);
        while (getchar() != '\n');

        sprintf(msg.order, "insert into %s values('%s','%s','%s','离线',0);", tableName, acount, name, password);
        //printf("%s\n", msg.order);
        sprintf(msg.fromName,"%s",acount);
        TcpClientSend(c, &msg, sizeof(msg));

        memset(msg.order,0,sizeof(msg.order));
        
        sleep(2);

        printf("=====噜啦噜啦,点击2按钮即可登录啦====\n");
        printf("===========按1重新输入哦!==========\n");
        scanf("%s", num); // 从函数里跳到了外面
        while (getchar() != '\n');
        
    }
    if (strcmp(num, "2") == 0)//登录
    {
        msg.flag = LOGIN;
        system("clear");
        printf("好久没见了，我还以为你把我忘了呢!\n");
        printf("=======输入账号哦============\n");
        scanf("%s", acount);
        while (getchar() != '\n');
        strcpy(msg.fromName,acount);
        
        printf("输入密码就可以进入游戏了，好开心\n");
        scanf("%s", password);
        while (getchar() != '\n');

        sprintf(msg.order, "select name from user where account = '%s' and password = '%s';",acount,password);
    
        printf("登录中~\n");
        sleep(3);
        TcpClientSend(c, &msg, sizeof(msg));
        
        memset(msg.order,0,sizeof(msg.order));
        sleep(1);


        while(1)
        {
            printf("========请输入你要执行的指令!======\n");
            printf("============1发送消息!===========\n");
            printf("============2查看通知!===========\n");
            printf("=3发送好友请求!      4接受好友请求!=\n");
            printf("============5进入世界频道!========\n");
            printf("============6显示在线好友!========\n");
            printf("=========7开始匹配狂扁小怪兽!======\n");
            printf("=============8我的群聊===========\n");
            printf("=9:创建群聊  10:拉人  11:发送群消息==\n");
            printf("=========12发文件   13接收文件=====\n");

            scanf("%s",num);
            while(getchar() != '\n');

            if(strcmp(num,"1") == 0)
            {
                msg.flag = SEND_MESSAGE;
                printf("请输入要发送的好友昵称!\n");
                scanf("%s",msg.toName);
                while(getchar() != '\n');
                
                while(1)
                {
                    printf("请输入要送送的消息!\n按1推出\n");
                    scanf("%s",msg.content);
                    while(getchar() != '\n');
                    
                    if(strcmp(msg.content,"1") == 0)
                        break;

                    TcpClientSend(c, &msg, sizeof(msg));
                }
                memset(msg.toName,0,sizeof(msg.toName));
                memset(msg.content,0,sizeof(msg.content));
            }
            if(strcmp(num,"2") == 0)
            {
                printf("请选择你要查看的消息通知\n");
                printf("===1查看所有通知,2按条查看未读通知\n");
                scanf("%s",num);
                while(getchar() != '\n');

                if(strcmp(num,"1") == 0)
                {
                    msg.flag = READ_ALL_INFO;
                    TcpClientSend(c,&msg,sizeof(msg));
                    printf("%d\n",msg.flag);
                }
                while(strcmp(num,"2") == 0)
                {
                    msg.flag = READ_GINGLE_INFO;
                    TcpClientSend(c,&msg,sizeof(msg));
                    printf("按2继续查看,q退出查看\n");
                    scanf("%s",num);
                    // while(getchar() != '\n');
                    if(strcmp(num,"q") == 0)
                    {
                        break;
                    }
                }
                if(strcmp(num,"1") != 0 && strcmp(num,"2") != 0)
                {
                    continue;
                }
            }
            if(strcmp(num,"3") == 0)//加好友
            {
                msg.flag = ADD_FRIENDS;
                printf("输入想添加的好用名\n");
                memset(msg.toName,0,sizeof(msg.toName));
                scanf("%s",msg.toName);
                TcpClientSend(c,&msg,sizeof(msg));
            }
            if(strcmp(num,"4") == 0)
            {
                memset(msg.toName,0,sizeof(msg.toName));
                msg.flag = AGREE_FRIENDS_APPLY;
                printf("请输入你要接受好友的ID\n");
                scanf("%s",msg.toName);
                TcpClientSend(c,&msg,sizeof(msg));
            }
            // if(strcmp(num,"6") == 0)
            // {
            //     msg.flag = 15;
            //     TcpClientSend(c,&msg,sizeof(msg));
            // }
            // if(strcmp(num,"8") == 0)//查看群聊列表
            // {
            //     msg.flag = 8;
            //     break;
            // }
            if(strcmp(num,"9") == 0)//创群
            {
                msg.flag = CREATE_GROUP;
                printf("请输入要创建的群聊名称\n");
                scanf("%s",msg.content);
                while(getchar() != '\n');
    
                TcpClientSend(c,&msg,sizeof(msg));
                sleep(1);
            }
            if(strcmp(num,"10") == 0)//拉人
            {
                msg.flag = GROUP_ADD;
                printf("请输入你要选择的群\n");
                scanf("%s",msg.content);
                while(getchar() != '\n');
                printf("请输入你的好友id\n");
                scanf("%s",msg.toName);
                while(getchar() != '\n');
                TcpClientSend(c,&msg,sizeof(msg));
            }
            if(strcmp(num,"11") == 0)//发送群消息
            {
                msg.flag = GROUP_SEND_MESSAGE;
                printf("请选择要发送的群名称\n");
                scanf("%s",msg.toName);
                while(getchar() != '\n');
                while(1)
                {
                    printf("1输入要发送的内容,2退出发送\n");
                    scanf("%s",num);
                    while(getchar() != '\n');

                    if(strcmp(num,"1") == 0)
                    {
                        scanf("%s",msg.content);
                        while(getchar() != '\n');
                        TcpClientSend(c,&msg,sizeof(msg));
                    }
                    if(strcmp(num,"2") == 0)
                    {
                        break;
                    }
                    if(strcmp(num,"1") != 0 && strcmp(num,"2") != 0)
                    {
                        continue;
                    }
                }
            }
            if(strcmp(num,"12") == 0)
            {
                msg.flag = SEND_FILE;
                printf("请输入你要发送的好友id\n");
                scanf("%s",msg.toName);
                while(getchar() != '\n');
                
                system("ls ./");
                printf("请输入你要发送的文件名\n");
                
                scanf("%s",msg.content);
                TcpClientSend(c,&msg,sizeof(msg));
            }
            if(strcmp(num,"13") == 0)
            {
                msg.flag = RECV_FILE;
                printf("请输入要接受的文件名\n");
                scanf("%s",msg.content);
                TcpClientSend(c,&msg,sizeof(msg));
                sleep(3);
                if(msg1.flag == 13)
                {
                    char newName[10]={0};
                    printf("请输入新的文件名：");
                    scanf("%s",newName);
                    WriteToFile(newName,msg1.content,msg1.back);
                }
            }
        }
    }
    //**************************************************************************

    // msg.flag = 1;
    // printf("请输入你的账号：\n");
    // scanf("%[^\n]", msg.fromName);
    // while (getchar() != '\n');
    // TcpClientSend(c, &msg, sizeof(msg));

    // while (1)
    // {
        // char temp[100] = {0};
    //     printf("1.单独发 2.群发\n");
    //     scanf("%d", &msg.flag);
    //     while (getchar() != '\n')
    //         ;
    //     msg.flag += 1;
    //     if (msg.flag == 2)
    //     {
    //         printf("请输入你需要发送的目标客户端账户：\n");
    //         scanf("%[^\n]", msg.toName);
    //         while (getchar() != '\n')
    //             ;

    //         printf("请输入你需要发送给目标客户端的内容：\n");
    //         scanf("%[^\n]", msg.content);
    //         while (getchar() != '\n')
    //             ;
    //         TcpClientSend(c, &msg, 100);
    //     }
    //     else
    //     {
    //         printf("请输入你需要发送给目标客户端的内容：\n");
    //         scanf("%[^\n]", msg.content);
    //         while (getchar() != '\n')
    //             ;
    //         TcpClientSend(c, &msg, 100);
    //     }
    // }
    // return 0;
}