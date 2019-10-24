

#ifndef __MYSOCK_H
#define __MYSOCK_H

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#ifndef BUFFER_SIZE
#define BUFFER_SIZE  4096
#endif

#ifndef ALIGNLENGHT
#define ALIGNLENGHT(len) ((((len)/BUFFER_SIZE)+((len)%BUFFER_SIZE? 1:0))*BUFFER_SIZE)
#endif

#ifdef __cplusplus
extern "C" {
#endif

BOOL myWSAStartup();
VOID myWSACleanup();
SOCKET myTcpSocket();
SOCKET myUdpSocket();
BOOL myConnect(SOCKET s, char * host, unsigned short port);
BOOL myBing(SOCKET s, int port);
BOOL myListen(SOCKET s, int backlog);
VOID myCloseSocket(SOCKET s);
int myRecv(SOCKET s, char ** buffer, CRITICAL_SECTION * section);
int mySend(SOCKET s,const char * buffer, int len,CRITICAL_SECTION * section);

#ifdef __cplusplus
}
#endif



#endif