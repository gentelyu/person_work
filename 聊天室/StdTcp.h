#ifndef __STDTCP_H_
#define __STDTCP_H_
#include<stddef.h>
typedef struct TcpServer TcpS;
TcpS* InitTcpServer(const char *IP,short int port);
int TcpServerAccept(TcpS *s);
void TcpServerSend(int ClientSock,void *sendBuffer,size_t sendBufferSize);
int TcpServerRecv(int ClientSock,void *recvBuffer,size_t recvBufferSize);
void ClearTcpServer(TcpS *s);

typedef struct TcpClient TcpC;
TcpC* InitTcpClient(const char *ServerIP,short int ServerPort);
void TcpClientSend(TcpC *c,void *sendBuffer,size_t sendBufferSize);
void TcpClientRecv(TcpC *c,void *recvBuffer,size_t recvBufferSize);
void ClearTcpClient(TcpC *c);
int GetTcpSock(TcpC *c);
#endif