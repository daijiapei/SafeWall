
#include "swssvc.h"

void InitUserList(PUSERLIST_HEADER UserList)
{
	UserList->header.Blink = &UserList->header;
	UserList->header.Flink = &UserList->header;
	InitializeCriticalSection(&UserList->section);//临界区
	UserList->icount =0;
}

void ReleaseUserList(PUSERLIST_HEADER UserList)
{
	UserList->header.Blink = &UserList->header;
	UserList->header.Flink = &UserList->header;
	DeleteCriticalSection(&UserList->section);
	UserList->icount =0;
}

long GetUserListCount(PUSERLIST_HEADER UserList, PPER_HANDLE_DATA pPerHandle)
{
	EnterCriticalSection(&UserList->section);
	LeaveCriticalSection(&UserList->section);
	return UserList->icount;
}

void SelectUserListNode(PUSERLIST_HEADER UserList)
{
	EnterCriticalSection(&UserList->section);
	LeaveCriticalSection(&UserList->section);
}

BOOL DeleteUserListNode(PUSERLIST_HEADER UserList, PPER_HANDLE_DATA pPerHandle)
{
	PER_HANDLE_DATA * node, * ptr = NULL;
	LIST_ENTRY *p;
	BOOL bRet;

	EnterCriticalSection(&UserList->section);

	if(UserList->header.Flink ==  &UserList->header ||
		(PPER_HANDLE_DATA)UserList->header.Flink > pPerHandle || 
		pPerHandle > (PPER_HANDLE_DATA)UserList->header.Blink)
	{
		//因为是从小到大排序的，如果pPerHandle不在集合中，则返回失败
		LeaveCriticalSection(&UserList->section);
		return FALSE;
	}

	p = UserList->header.Flink;
	while(p != &UserList->header)
	{
		node = (PPER_HANDLE_DATA)p;
		if(node == pPerHandle)
		{
			p->Blink->Flink = p->Flink;
			p->Flink->Blink = p->Blink;
			bRet = TRUE;
			--UserList->icount;
			break;
		}
		//因为是按顺序排的，如果pPerHandle大于节点，就表示不存在该pPerHandle
		if(pPerHandle > node)
		{
			bRet = FALSE;
			break;
		}
		p = p->Flink;
	}
	LeaveCriticalSection(&UserList->section);
	return bRet;
}

BOOL InserUserListNode(PUSERLIST_HEADER UserList,PPER_HANDLE_DATA pPerHandle )
{
	PER_HANDLE_DATA * node = NULL;
	LIST_ENTRY * p;
	BOOL has = FALSE;
	EnterCriticalSection(&UserList->section);

	if(UserList->header.Flink ==  &UserList->header ||
		(PPER_HANDLE_DATA)UserList->header.Flink > pPerHandle)
	{
		//传入的pPerHandle太小，将它放在第一位
		p =  &UserList->header;
	}
	else if(pPerHandle > (PPER_HANDLE_DATA)UserList->header.Blink)
	{
		//传入的pPerHandle太大，将他放在最后一位
		p =  UserList->header.Blink;
	}
	else
	{
		p = UserList->header.Flink;
		while(p != &UserList->header)
		{
			node = (PPER_HANDLE_DATA)p;
			if(node == pPerHandle)
			{
				has = TRUE;
				//pPerHandle已存在，这里是为了容错，按照逻辑我们是
				//不会发生插入两次的
				break;
			}
			//因为是按顺序排的，如果节点大于pPerHandle，
			//就表示这个节点适合加入pPerHandle
			if(node > pPerHandle)
			{
				break;
			}
			p = p->Flink;
		}
	}

	if(FALSE == has)//没有这个pPerHandle
	{
		pPerHandle->list.Flink = p->Flink;
		pPerHandle->list.Blink = p;

		p->Flink->Blink = (LIST_ENTRY*)pPerHandle;
		p->Flink = (LIST_ENTRY*)pPerHandle;
			
		++UserList->icount;
	}
	
	LeaveCriticalSection(&UserList->section);
	return !has;
}