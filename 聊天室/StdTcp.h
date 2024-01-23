#ifndef __STDTCP_H_
#define __STDTCP_H_
#include<stddef.h>
typedef struct TcpServer TcpS;
TcpS* InitTcpServer(const char *IP,short int port);
int TcpServerAccept(TcpS *s);
void TcpServerSend(int ClientSock,void *ptr,size_t size);
void TcpServerRecv(int ClientSock,void *ptr,size_t size);
void ClearTcpServer(TcpS *s);

typedef struct TcpClient TcpC;
TcpC* InitTcpClient(const char *ServerIP,short int ServerPort);
void TcpClientSend(TcpC *c,void *ptr,size_t size);
void TcpClientRecv(TcpC *c,void *ptr,size_t size);
void ClearTcpClient(TcpC *c);
int GetTcpSock(TcpC *c);
#endif