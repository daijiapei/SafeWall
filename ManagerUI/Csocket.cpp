
#include "csocket.h"


void CMyTcpSocket::WSAStartup()
{
	myWSAStartup();
}

void CMyTcpSocket::WSACleanup()
{
	myWSACleanup();
}

CMyTcpSocket::CMyTcpSocket()
{
	m_section = NULL;
	m_sock = myTcpSocket();
}

CMyTcpSocket::~CMyTcpSocket()
{
	if(m_sock)
	{
		closesocket(m_sock);
	}
	if(m_section)
	{
		DeleteCriticalSection(m_section);
		free(m_section);
	}
}

void CMyTcpSocket::create_section()
{
	m_section = (CRITICAL_SECTION *)malloc(sizeof(CRITICAL_SECTION));
	::InitializeCriticalSection(m_section);
}

void CMyTcpSocket::close()
{
	if(m_sock)
	{
		::closesocket(m_sock);
	}
}

BOOL CMyTcpSocket::connect(char * host, unsigned short port)
{
	return ::myConnect(m_sock, host, port);
}

int CMyTcpSocket::shutdown(int how)
{
	return ::shutdown(m_sock, how);
}

int CMyTcpSocket::WSAAsyncSelect(HWND hwnd, UINT uMsg, long lEvent)
{
	return ::WSAAsyncSelect(m_sock ,hwnd , uMsg, lEvent);
}

int CMyTcpSocket::recv(char ** buffer)
{
	return ::myRecv(m_sock,buffer, NULL);
}

int CMyTcpSocket::send(const char * buffer, int len)
{
	return ::mySend(m_sock, buffer, len, m_section);
}