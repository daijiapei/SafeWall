
#include "SafeWall.h"

MY_LIST_HERDER gProcessListHeader;       //涉密进程链表

#pragma PAGEDCODE
VOID InitializeMyProcessListHead()
{
	InitializeListHead((PLIST_ENTRY)&gProcessListHeader);//初始化文件链表
	KeInitializeSpinLock(&gProcessListHeader.locker);
	gProcessListHeader.icount = 0;
}
//
//#pragma PAGEDCODE
//SAFEWALL_FILE_LIST * SelectProcessListNode(PFSRTL_COMMON_FCB_HEADER pFcb)
//{
//	SAFEWALL_FILE_LIST * node, * ptr = NULL;
//	LIST_ENTRY *p;
//	KLOCK_QUEUE_HANDLE handle;
//
//	KeAcquireInStackQueuedSpinLock(&gFileListHeader.locker, &handle);
//	if(gFileListHeader.header.Flink ==  &gFileListHeader.header ||
//		((PSAFEWALL_FILE_LIST)gFileListHeader.header.Flink)->pFcb > pFcb || 
//		pFcb > ((PSAFEWALL_FILE_LIST)gFileListHeader.header.Blink)->pFcb )
//	{
//		/*因为是从小到大排序，先判断pfb是否在集合中,否则返回*/
//		KeReleaseInStackQueuedSpinLock(&handle);
//		return ptr;
//	}
//	p = gFileListHeader.header.Flink ;
//	while( p != &gFileListHeader.header)
//	{
//		node = (PSAFEWALL_FILE_LIST)p;
//		if(node->pFcb == pFcb)
//		{
//			//KdPrint(("%wZ 文件 fcb 索引查找成功", &node->FilePath));
//			ptr = node;
//			break;
//		}
//		//因为是按顺序排的，如果fcb大于节点，就表示不存在该fcb
//		if(pFcb > node->pFcb)
//		{
//			break;
//		}
//		p = p->Flink;
//	}
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return ptr;
//}
//
//#pragma PAGEDCODE
//BOOLEAN DeleteFileListNode(PFSRTL_COMMON_FCB_HEADER pFcb)
//{
//	BOOLEAN dwRet = FALSE;
//	SAFEWALL_FILE_LIST * node;
//	LIST_ENTRY *p;
//	KLOCK_QUEUE_HANDLE handle;
//
//	KeAcquireInStackQueuedSpinLock(&gFileListHeader.locker, &handle);
//	if(gFileListHeader.header.Flink ==  &gFileListHeader.header ||
//		((PSAFEWALL_FILE_LIST)gFileListHeader.header.Flink)->pFcb > pFcb || 
//		pFcb > ((PSAFEWALL_FILE_LIST)gFileListHeader.header.Blink)->pFcb)
//	{
//		//因为是从小到大排序的，如果fcb不在集合中，则返回失败
//		KeReleaseInStackQueuedSpinLock(&handle);
//		return FALSE;
//	}
//
//	p = gFileListHeader.header.Flink;
//	while(p != &gFileListHeader.header)
//	{
//		node = (PSAFEWALL_FILE_LIST)p;
//		if(node->pFcb == pFcb)
//		{
//			p->Blink->Flink = p->Flink;
//			p->Flink->Blink = p->Blink;
//			ExFreePool(node);
//			dwRet = TRUE;
//			--gFileListHeader.icount;
//			break;
//		}
//		//因为是按顺序排的，如果fcb大于节点，就表示不存在该fcb
//		if(pFcb > node->pFcb)
//		{
//			dwRet = FALSE;
//			break;
//		}
//		p = p->Flink;
//	}
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return dwRet;
//}
//
//#pragma PAGEDCODE
//SAFEWALL_FILE_LIST * InsertSingleFileListNode(PFSRTL_COMMON_FCB_HEADER pFcb, BOOLEAN * IsHas)
//{
//	//如果fcb已经存在，则不会插入，但是会返回对应的链表指针，ishas也会返回TRUE
//	SAFEWALL_FILE_LIST * node = NULL;
//	SAFEWALL_FILE_LIST * newNode =NULL;
//	LIST_ENTRY *p = NULL;
//	KLOCK_QUEUE_HANDLE handle;
//	* IsHas = FALSE; //是否已经存在该fcb
//	KeAcquireInStackQueuedSpinLock(&gFileListHeader.locker, &handle);
//	if(gFileListHeader.header.Flink ==  &gFileListHeader.header ||
//		((PSAFEWALL_FILE_LIST)gFileListHeader.header.Flink)->pFcb > pFcb)
//	{
//		//KdPrint(("fcb = 0x%08x 传入的fcb太小，将它放在第一位",pFcb));
//		p =  &gFileListHeader.header;
//	}
//	else if(pFcb > ((PSAFEWALL_FILE_LIST)gFileListHeader.header.Blink)->pFcb)
//	{
//		//KdPrint(("fcb = 0x%08x 传入的fcb太大，将他放在最后一位",pFcb));
//		p =  gFileListHeader.header.Blink;
//	}
//	else
//	{
//		p = gFileListHeader.header.Flink;
//		while(p != &gFileListHeader.header)
//		{
//			node = (PSAFEWALL_FILE_LIST)p;
//			if(node->pFcb == pFcb)
//			{
//				newNode = node;
//				* IsHas = TRUE;
//				//KdPrint(("fcb = 0x%08x fcb已存在",pFcb));
//				break;
//			}
//			//因为是按顺序排的，如果节点大于fcb，
//			//就表示这个节点适合加入pcb
//			if(node->pFcb > pFcb)
//			{
//				//KdPrint(("node->pFcb = 0x%08x AND pFcb = 0x%08x ",node->pFcb , pFcb));
//				break;
//			}
//			p = p->Flink;
//		}
//	}
//	
//	if(FALSE == *IsHas)//没有这个fcb
//	{
//		newNode = (SAFEWALL_FILE_LIST*)ExAllocatePool(NonPagedPool, sizeof(SAFEWALL_FILE_LIST));
//		if(NULL != newNode)
//		{
//			newNode->pFcb = pFcb;
//			RtlInitEmptyUnicodeString(&newNode->FilePath, newNode->wstr, sizeof(newNode->wstr));
//
//			newNode->list.Flink = p->Flink;
//			newNode->list.Blink = p;
//
//			p->Flink->Blink = (LIST_ENTRY*)newNode;
//			p->Flink = (LIST_ENTRY*)newNode;
//			
//			++gFileListHeader.icount;
//			KdPrint(("新的fcb插入到链表中：0x%08x, 累计：%d", pFcb,gFileListHeader.icount));
//		}
//	}
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return newNode;
//
//}
//
//#pragma PAGEDCODE
//VOID ReleaseAllFileListNode()
//{
//	LIST_ENTRY * node;
//	//for(node = gFileListHeader.Flink; node != &gFileListHeader; node = node->Flink)
//	//{
//	//	RemoveEntryList((PLIST_ENTRY)node);
//	//	ExFreePool(node);
//	//}
//}