#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "StdTcp.h"

//TCP服务器
struct TcpServer
{
    int sock;
};
//初始化服务器
TcpS *InitTcpServer(const char *IP, short int port)
{
    TcpS * s = (TcpS *)malloc(sizeof(TcpS));
    if(s == NULL)
    {
        printf("InitTcpServer malloc error!\n");
        return NULL;
    }
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
    struct sockaddr_in addr;//定义了一个sockaddr_in结构体变量addr，用于存储服务器的地址信息。
    addr.sin_family = AF_INET;//设置addr结构体的sin_family成员为AF_INET，表示使用IPv4协议。
    addr.sin_port = htons(port);//将传入的参数port转换为网络字节序，并赋值给addr结构体的sin_port成员，表示服务器的端口号。
    addr.sin_addr.s_addr = inet_addr(IP);//将传入的参数IP转换为网络字节序的IP地址，并赋值给addr结构体的sin_addr.s_addr成员，表示服务器的IP地址。

    if(bind(s->sock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
    //调用bind函数将套接字绑定到指定的地址上。
    {
        perror("bind:");
        free(s);
        return NULL;
    }

    if(listen(s->sock,10) < 0)
    //调用listen函数开始监听客户端连接请求。第二个参数10表示同时有多少个客户端可以连接到服务器。
    {
        perror("listen:");
        free(s);
        return NULL;
    }

    return s;
}

//将已经握手的客户端
int TcpServerAccept(TcpS *s)
{
    int acceptSock = 0;
    struct sockaddr_in addr;
    socklen_t len = 0; 
    if((acceptSock = accept(s->sock,(struct sockaddr*)&addr,&len)) < 0)
//调用accept函数接受客户端的连接请求，并将返回的套接字赋值给acceptSock变量。
//accept函数的第一个参数是服务器的监听套接字s->sock，
//第二个参数是指向客户端地址的指针，第三个参数是指向len的指针，表示传入传出参数。
    {
        perror("accept:");
        return -1;
    }
    return acceptSock;//返回接受到的与客户端通信的套接字
}

void TcpServerSend(int ClientSock, void *ptr, size_t size)
{
    if (send(ClientSock,ptr,size,0) < 0)
    {
        perror("send");
    }
}

void TcpServerRecv(int ClientSock, void *ptr, size_t size)
{
    if (recv(ClientSock,ptr,size,0) < 0)
    {
        perror("recv");
    }
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
        perror("connect:");
        free(c);
        return NULL;
    }
    return c;
}

void TcpClientSend(TcpC *c, void *ptr, size_t size)
{
    if(send(c->sock,ptr,size,0) < 0)
    {
        perror("send");
    }
}

void TcpClientRecv(TcpC *c, void *ptr, size_t size)
{
    if(recv(c->sock,ptr,size,0) < 0)
    {
        perror("recv");
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
