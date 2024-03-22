#ifndef __STDTCP_H_
#define __STDTCP_H_
#include<stddef.h>
#include "StdSqlite.h"
#include "ThreadPool.h"
#include <sys/epoll.h>

#define BUFFER_SIZE 128

//TCP服务器
typedef struct TcpServer
{
    int sock;
    int communicate_sock;
    int epoll_fd;
    struct epoll_event events[BUFFER_SIZE];
    ThreadP * pThreadPool;

    /* 新增参数数据库结构体 */
    SQL *sql;
} TcpS;

TcpS* InitTcpServer(const char *IP,short int port);
int TcpServerAccept(TcpS *s, void *(*func)(void *), void *arg2);
void TcpServerSend(int ClientSock,void *sendBuffer,size_t sendBufferSize);
int TcpServerRecv(TcpS *s, int ClientSock,void *recvBuffer,size_t recvBufferSize);
void ClearTcpServer(TcpS *s);

typedef struct TcpClient TcpC;
TcpC* InitTcpClient(const char *ServerIP,short int ServerPort);
void TcpClientSend(TcpC *c,void *sendBuffer,size_t sendBufferSize);
void TcpClientRecv(TcpC *c,void *recvBuffer,size_t recvBufferSize);
void ClearTcpClient(TcpC *c);
int GetTcpSock(TcpC *c);
#endif