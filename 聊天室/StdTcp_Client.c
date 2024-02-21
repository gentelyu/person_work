#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "StdTcp_Client.h"



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