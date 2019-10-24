
#include "..\\include\\mysock.h"


class CMyTcpSocket 
{
public:
	static void WSAStartup();
	static void WSACleanup();

public:
	CMyTcpSocket();
	~CMyTcpSocket();

	void create_section();
	void close();

	BOOL connect(char * host, unsigned short port);
	int recv(char ** buffer);
	int send(const char * buffer, int len);

	int shutdown(int how);
	int WSAAsyncSelect(HWND hwnd, UINT uMsg, long lEvent);
	
private:
	SOCKET m_sock;
	CRITICAL_SECTION * m_section;
};

