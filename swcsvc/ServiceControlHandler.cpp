
#include "swcsvc.h"
#include "..\\include\\inirw.h"
#include "..\\include\\assist.h"
#include <time.h>

SOCKET server = NULL;
char userid[64] = {0};//普通用户
char password[64] = {0};
char userid2[64] = {0};//IC用户
char password2[64] = {0};

time_t LastTime; //这个是秒数
int EffectiveTime=0; //证书有效时间,一般最低限时5分钟

#define MIN_EFFECTIVE_TIME (5*60)//5分钟

#define SYNC_NO_ERROR     0
#define SYNC_SEND_ERROR   1
#define SYNC_READ_ERROR   2
#define SYNC_SOCKET_VALUE 3

//心跳包
UINT WINAPI KeepActivityThread(LPVOID lpParam)
{
	long data = 0;
	MYSERVCONTEXT Context;
	memset(&Context,0, sizeof(MYSERVCONTEXT));
	while(running)
	{
		Sleep(28 * 1000);//休息28秒
		if(userid[0]!='\0')
		{
			if(SOCKET_ERROR == mySend(server,(char*)&data, sizeof(data), &gSection))
			{
				//发送心跳包失败
				closesocket(server);
				server = NULL;
				strcpy(Context.userid,userid);
				strcpy(Context.password, password);
				OnLogon(&Context);
			}
			else
			{
				LastTime = time(NULL);
			}
		}
	}
	return 0;
}

DWORD OnLogon(LPMYSERVCONTEXT lpContext)
{
	char * buffer;
	SOCKADDR_IN servAddr;
	Json::Value root; 
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;
	std::string sentbuf;
	DWORD error;

	char ipaddr[256]= "127.0.0.1";
	int port = 1808;
	
	if(server && userid[0]!='\0')
	{
		//已经有用户登录
		//mylog(LogPath,"已经有用户登录");
		return 1;
	}

	INIOBJECT iniobj =  CreateIniObject(szConfigFile);
	iniGetString(iniobj,"safewall server", "ipaddr", ipaddr, sizeof(ipaddr),"127.0.0.1");
	port = iniGetInt(iniobj,"safewall server", "port", port);
	ReleaseIniObject(iniobj);
	error = SOCKET_ERROR;
	do
	{
		server = myTcpSocket();

		if(FALSE == myConnect(server, ipaddr, port))
		{
			//mylog(szLogPath,"连接服务器失败");
			break;
		}
	
		context["userid"] = lpContext->userid;
		context["password"] = lpContext->password;
		root["command"] = SAFEWALL_CLIENT_LOGON;
		root["context"] = context;
		sentbuf = root.toStyledString();

		if(SOCKET_ERROR == mySend(server,(char*)sentbuf.c_str(), sentbuf.length(), &gSection))
		{
			break;
		}

		if(SOCKET_ERROR == myRecv(server, &buffer, NULL))
		{
			break;
		}

		if(true == reader.parse(buffer,reply))
		{
			free(buffer);
			if(0 == stricmp(reply["state"].asCString(),"success"))
			{
				//登录成功，接下来来获取策略
				strcpy(userid, lpContext->userid);
				strcpy(password, lpContext->password);
				LastTime = time(NULL);
				return OnGetAccess();
			}
		}
		//到这里，就是连接成功，但是账号密码不正确之类的
		server = NULL;
		userid[0] = '\0';
		userid2[0] = '\0';
	}while(FALSE);
	
	return error;
}

DWORD OnGetUserID(LPMYSERVCONTEXT lpContext)
{
	BOOL timeout = FALSE;
	if(EffectiveTime > 0)
	{
		timeout = (time(NULL) - LastTime) > EffectiveTime ? TRUE : FALSE;
	}

	if(userid[0] && !timeout)
	{
		//用户已登录，并且未发生断线超时
		strcpy(lpContext->userid, userid);
		return 0;
	}
	return 1;
}

DWORD OnCancelLogon(void)
{
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;
	DWORD error;
	char * buffer;

	error = 1;//无论如果，一定是注销成功的
	do
	{
		if(NULL == server)
		{
			//没有用户登录
			break;
		}
		root["command"] = SAFEWALL_CLIENT_CANCEL_LOGON;
		context["userid"] = userid;//忽略
		root["context"] = context;
		std::string sentbuf = root.toStyledString();

		if(SOCKET_ERROR == mySend(server,(char*)sentbuf.c_str(), sentbuf.length(), &gSection))
		{
			break;
		}
		shutdown(server,0);//关闭读
		error = 0;
	}while(FALSE);
	closesocket(server);
	//返回什么都不重要了，直接注销
	userid[0] = '\0';
	userid2[0] = '\0';
	password[0] = '\0';
	
	server = NULL;
	//通知加密
	return error;
}

DWORD OnChangePassword(LPMYSERVCONTEXT lpContext)
{
	Json::Value root; 
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;
	DWORD error;
	char * buffer;

	context["userid"] = lpContext->userid;
	context["password"] = lpContext->password;
	context["newpwd"] = lpContext->newpwd;
	root["command"] = SAFEWALL_CLIENT_CHANGE_PASSWORD;
	root["context"] = context;
	std::string sentbuf = root.toStyledString();

	if(SOCKET_ERROR == mySend(server,(char*)sentbuf.c_str(), sentbuf.length(), &gSection))
	{
		return SOCKET_ERROR;
	}

	if(SOCKET_ERROR == myRecv(server, &buffer, NULL))
	{
		return SOCKET_ERROR;
	}

	if(true == reader.parse(buffer,reply))
	{
		free(buffer);
		if(!stricmp(reply["state"].asCString(),"success"))
		{
			//修改成功
			strcpy(password, lpContext->newpwd);
			return 0;
		}
	}
	
	return -2;
}

DWORD OnGetAccess(void)
{
	Json::Value root;
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;
	Json::Value temp;
	DWORD error;
	char *buffer;

	context["userid"] = userid;//忽略
	root["command"] = SAFEWALL_CLIENT_LOAD_ACCESS;
	root["context"] = context;
	std::string sentbuf = root.toStyledString();

	if(SOCKET_ERROR == mySend(server,(char*)sentbuf.c_str(), sentbuf.length(), &gSection))
	{
		goto end;
	}

	if(SOCKET_ERROR == myRecv(server, &buffer, NULL))
	{
		goto end;
	}
	
	if(true == reader.parse(buffer,reply))
	{
		//mylog(szLogFile,buffer);
		free(buffer);

		temp = reply["access"]["EffectiveTime"];
		if(temp.isNull() || 0 == temp.asInt())
		{
			EffectiveTime = 0;//不限时
		}
		else if(MIN_EFFECTIVE_TIME > temp.asInt())
		{
			EffectiveTime = MIN_EFFECTIVE_TIME;
		}
		else
		{
			EffectiveTime = temp.asInt();
		}

		return 0;
	}
	
end:
	return SOCKET_ERROR;
}
