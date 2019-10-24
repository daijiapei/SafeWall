
#include "mysock.h"
#include <Windows.h>
BOOL myWSAStartup()
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) == 0) return TRUE;
	return FALSE;
}

VOID myWSACleanup()
{
	WSACleanup();
}

SOCKET myTcpSocket()
{
	int reuse = 1;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR ,(char*)&reuse, sizeof(reuse));
	return s;
}

SOCKET myUdpSocket()
{
	SOCKET s = socket(AF_INET,SOCK_DGRAM,0);
}

BOOL myConnect(SOCKET s, char * host, unsigned short port)
{
	SOCKADDR_IN si;
	struct hostent * pHostent;

	pHostent = gethostbyname(host);
	if(NULL == pHostent || NULL == pHostent->h_addr_list[0])
	{
		return FALSE;
	}
	memset(&si, 0, sizeof(SOCKADDR_IN));
	si.sin_family = AF_INET;
	si.sin_addr = *(struct in_addr *)pHostent->h_addr_list[0];
	si.sin_port = htons(port);
	
	if(SOCKET_ERROR == connect(s, (SOCKADDR*)&si, sizeof(SOCKADDR_IN)))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL myBing(SOCKET s, int port)
{
	SOCKADDR_IN si;
	si.sin_family = AF_INET;
	si.sin_port = htons(port);
	si.sin_addr.S_un.S_addr = INADDR_ANY;
	
	if(SOCKET_ERROR == bind(s,(struct sockaddr *)&si, sizeof(si)))
	{
		return FALSE;
	}
	return TRUE;

}

BOOL myListen(SOCKET s, int backlog)
{
	if(0 == backlog)
	{
		backlog = 199;//window×î´ó¼àÌý200
	}

	if(SOCKET_ERROR == listen(s, backlog))
	{
		return FALSE;
	}
	return TRUE;
}

VOID myCloseSocket(SOCKET s)
{
	closesocket(s);
}

int mySend(SOCKET s,const char * buffer, int len,CRITICAL_SECTION * section)
{
	int size,offset, ret;

	if(section) EnterCriticalSection(section);
	size = htonl(len);
	ret = send(s,(char *) &size, sizeof(size), 0);
	size = len;
	while(ret != SOCKET_ERROR)
	{
		offset = (size > BUFFER_SIZE) ? BUFFER_SIZE : size;
		ret = send(s, buffer, offset, 0);
		size -= ret;
		buffer += ret;
		if(size == 0)
		{
			ret = len;
			break;
		}
	}
	if(section) LeaveCriticalSection(section);

	return ret;
}

int myRecv(SOCKET s, char ** buffer, CRITICAL_SECTION * section)
{
	int size,offset,len, ret;
	char *p;
	if(section) EnterCriticalSection(section);
	ret = recv(s, (char*)&len, sizeof(len),0);
	if(ret != SOCKET_ERROR)
	{
		ret = SOCKET_ERROR;
		len = ntohl(len);
		size = len;
		p = (char*)malloc(ALIGNLENGHT(len));
		*buffer = p;
		while(*buffer)
		{
			offset = (size > BUFFER_SIZE) ? BUFFER_SIZE : size;
			ret = recv(s, p, offset, 0);
			size -= ret;
			p += ret;
			if(ret == SOCKET_ERROR)
			{
				free(*buffer);
				break;
			}

			if(size == 0)
			{
				*p = '\0';
				//*buffer[len] = '\0';
				ret = len;
				break;
			}
		}
	}

	if(section) LeaveCriticalSection(section);
	return ret;
}