#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "StdTcp.h"
// #include "ThreadPool.h"
#include <fcntl.h>
#include <errno.h>


#define LISTEN_QUEUE_LENGTH 128
#define false -1

#define READ_BUFFER_SIZE 10


//初始化服务器
TcpS *InitTcpServer(const char *IP, short int port)
{
    TcpS * s = (TcpS *)malloc(sizeof(TcpS));
    if(s == NULL)
    {
        printf("InitTcpServer malloc error!\n");
        return NULL;
    }
    //清除脏数据
    memset(s, 0, sizeof(TcpS));
    s->sock = socket(AF_INET,SOCK_STREAM,0);
    //调用socket函数创建一个套接字。AF_INET表示使用IPv4协议，SOCK_STREAM表示使用TCP协议，0表示选择默认的协议。
    if(s->sock < 0)
    {
        perror("socket():");
        free(s);
        return NULL;
    }
    
    int on = 1;
    if(setsockopt(s->sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int)) < 0)
    //调用setsockopt函数设置套接字选项，将套接字设置为可以重用地址。如果设置失败，则打印错误信息，并释放之前分配的内存，返回NULL。
    {
        perror("setsockopt");
        free(s);
        return NULL;
    }
    struct sockaddr_in addr;                //定义了一个sockaddr_in结构体变量addr，用于存储服务器的地址信息。
    addr.sin_family = AF_INET;              //设置addr结构体的sin_family成员为AF_INET，表示使用IPv4协议。
    addr.sin_port = htons(port);            //将传入的参数port转换为网络字节序，并赋值给addr结构体的sin_port成员，表示服务器的端口号。
    addr.sin_addr.s_addr = inet_addr(IP);   //将传入的参数IP转换为网络字节序的IP地址，并赋值给addr结构体的sin_addr.s_addr成员，表示服务器的IP地址。

    if(bind(s->sock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
    //调用bind函数将套接字绑定到指定的地址上。
    {
        perror("bind:");
        free(s);
        return NULL;
    }

    if(listen(s->sock,LISTEN_QUEUE_LENGTH) < 0)
    //调用listen函数开始监听客户端连接请求。第二个参数10表示同时有多少个客户端可以连接到服务器。
    {
        perror("listen:");
        free(s);
        return NULL;
    }


    //创建epoll 红黑树 实例
    s->epoll_fd = epoll_create(1);
    if (s->epoll_fd == -1)
    {
        perror("epfd error");
        exit(-1);
    }

    //将sockfd 添加到红黑树实例里面
    struct epoll_event event;
    /* 清除脏数据 */
    memset(&event, 0, sizeof(event));
    event.data.fd = s->sock;
    event.events = EPOLLIN;     // 读事件
    int ret = epoll_ctl(s->epoll_fd, EPOLL_CTL_ADD, s->sock, &event);
    if (ret == -1)
    {
        perror("epoll ctl error");
        exit(-1);
    }


    return s;
}




int TcpServerAccept(TcpS *s, void *(*func)(void *), void *arg2)
{
#if 1

    //监听到的数量
    int nums = 0;
    //读到的字节数
    int readBytes = 0;

    while(1)
    {
        nums = epoll_wait(s->epoll_fd, s->events, BUFFER_SIZE, -1);
        if (nums == -1)
        {
            perror("epoll_wait error");
            exit(-1);
        }

        printf("epoll_wait 监听到：%d个文件描述符有变化\n", nums);

        for (int idx = 0; idx < nums; idx++)
        {
            int this_fd = s->events[idx].data.fd;
            if (this_fd == s->sock)
            {
                //有连接
                int connfd = accept(s->sock, NULL, NULL);
                if (connfd == -1)
                {
                    perror("accept error");
                    exit(-1);
                }

                //将通信句柄this_fd 设置呈非阻塞模式
                int flag = fcntl(connfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(connfd, F_SETFL, flag);

                struct epoll_event conn_event;
                memset(&conn_event, 0, sizeof(conn_event));
                conn_event.data.fd = connfd;
                //将默认的水平触发模式改成边沿触发模式
                conn_event.events = EPOLLIN | EPOLLET;

                //将通信句柄添加到树节点中
                int ret = epoll_ctl(s->epoll_fd, EPOLL_CTL_ADD, connfd, &conn_event);
                if (ret == -1)
                {
                    perror("epoll_ctl error");
                    continue;
                }

                /*
                打印...上线
                */

            }
            else
            {
                //有客户端通信
                #if 0
                while(1)
                {

                    char buffer[READ_BUFFER_SIZE] = { 0 };
                    
                    readBytes = read(this_fd, buffer, sizeof(buffer) - 1);
                    if (readBytes < 0)
                    {
                        if (errno == EAGAIN)
                        {
                            /* todo...*/
                            printf("read end...\n");
                            break;
                        }
                        else
                        {
                            /* 将文件句柄从 红黑树上删掉 */
                            epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, this_fd, NULL);
                            /* 关闭文件句柄 */
                            close(this_fd);
                            break;
                        }
                    }
                    else if (readBytes)
                }

                #else
                /* 交给线程池处理 */
                s->communicate_sock = this_fd;
                Threadp_AddTask(s->pThreadPool, func, arg2);
                
                #endif
            }
        }
    }


#else

    int acceptSock = 0;
    struct sockaddr_in addr;
    socklen_t clientAddressLen = 0;
    if((acceptSock = accept(s->sock,(struct sockaddr*)&addr,&clientAddressLen)) < 0)
//调用accept函数接受客户端的连接请求，并将返回的套接字赋值给acceptSock变量。
//accept函数的第一个参数是服务器的监听套接字s->sock，
//第二个参数是指向客户端地址的指针，第三个参数是指向clientAddressLen的指针，表示传入传出参数。
    {
        perror("accept:");
        return false;
    }
    return acceptSock;//返回接受到的与客户端通信的套接字
#endif
}

void TcpServerSend(int ClientSock, void *sentBuffer, size_t sendBufferSize)
{
    if (send(ClientSock,sentBuffer,sendBufferSize,0) < 0)
    {
        perror("send");
    }
}

int TcpServerRecv(TcpS *s, int ClientSock, void *recvBuffer, size_t recvBufferSize)
{
    int ret = 0;
    ret = recv(ClientSock, recvBuffer, recvBufferSize, 0);
    if (ret < 0)
    {
        if (errno == EAGAIN)
        {
            printf("read end...\n");
        }
        else
        {
            /* 将该文件句柄 从红黑树上删掉 */
            epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, ClientSock, NULL);
            close(ClientSock);
        }
        // perror("recv error");
    }
    else if (ret == 0)
    {
        printf("客户端 断开连接\n");
        /* 将该文件句柄 从红黑树上删掉 */
        epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, ClientSock, NULL);
        /* 关闭该客户端通信句柄 */
        close(ClientSock);
    }

    return ret;
}

void ClearTcpServer(TcpS *s)
{
    close(s->sock);
    free(s);
}



struct TcpClient
{
    int sock;
};

TcpC *InitTcpClient(const char *ServerIP, short int ServerPort)//初始化结构体，接受两个参数，服务器IP地址、端口号。
{
    TcpC * c = (TcpC * )malloc(sizeof(TcpC));
    if(c == NULL)
    {
        printf("InitTcpServer malloc error!\n");
        return NULL;
    }
    c->sock = socket(AF_INET,SOCK_STREAM,0);//设置套接字属性，IPV4协议，TCP协议，默认属性
    if(c->sock < 0)
    {
        perror("socket():");
        free(c);
        return NULL;
    }

    struct sockaddr_in ServerAddr;//
    ServerAddr.sin_family = AF_INET;//设置服务器地址结构体ServerAddr的sin_family成员变量为AF_INET，表示使用IPv4地址。
    ServerAddr.sin_port = htons(ServerPort);//将提供的服务器端口号ServerPort转换为网络字节序（大端字节序）并赋值给ServerAddr.sin_port，表示要连接的服务器端口号。
    ServerAddr.sin_addr.s_addr = inet_addr(ServerIP);
    //将提供的服务器IP地址ServerIP转换为网络字节序的二进制IP地址，并将其赋值给ServerAddr.sin_addr.s_addr，表示要连接的服务器IP地址。
    if(connect(c->sock,(struct sockaddr *)&ServerAddr,sizeof(ServerAddr)) < 0)
    //使用connect函数连接到指定的服务器。
    //第一个参数是客户端套接字文件描述符c->sock，第二个参数是指向服务器地址结构体的指针，第三个参数是服务器地址结构体的大小。
    {
        perror("connect error");
        free(c);
        return NULL;
    }
    return c;
}

void TcpClientSend(TcpC *c, void *sendBuffer, size_t sendBufferSize)
{
    if(send(c->sock, sendBuffer, sendBufferSize,0) < 0)
    {
        perror("send error");
    }
}

void TcpClientRecv(TcpC *c, void *recvBuffer, size_t recvBufferSize)
{
    // if(recv(c->sock,ptr,size,0) < 0)
    // {
    //     perror("recv:");
    // }
    int ret = 0;
    ret = recv(c->sock, recvBuffer, recvBufferSize, 0);
    if (ret < 0)
    {
        perror("recv error");
    }
    else if (ret == 0)
    {
        printf("404");
        exit(-1);
    }
}

void ClearTcpClient(TcpC *c)
{
    close(c->sock);
    free(c);
}

int GetTcpSock(TcpC *c)
{
    return c->sock;
}
