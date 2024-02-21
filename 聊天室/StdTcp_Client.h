#ifndef __STDTCP_CLIENT_H_
#define __STDTCP_CLIENT_H_
#include<stddef.h>

typedef struct TcpClient TcpC;
TcpC* InitTcpClient(const char *ServerIP,short int ServerPort);
void TcpClientSend(TcpC *c,void *sendBuffer,size_t sendBufferSize);
void TcpClientRecv(TcpC *c,void *recvBuffer,size_t recvBufferSize);
void ClearTcpClient(TcpC *c);
int GetTcpSock(TcpC *c);

#endif