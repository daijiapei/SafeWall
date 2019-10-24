

#include "SafeWall.h"


// 清理缓冲
#pragma PAGEDCODE
void FileSystemCacheClear(PFILE_OBJECT pFileObject)
{
   PFSRTL_COMMON_FCB_HEADER pFcb;
   LARGE_INTEGER liInterval;
   BOOLEAN bNeedReleaseResource = FALSE;
   BOOLEAN bNeedReleasePagingIoResource = FALSE;
   KIRQL irql;
   PFCB p;

   pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
   if(pFcb == NULL)
       return;

   //返回当前中断级别
   irql = KeGetCurrentIrql();
   if (irql >= DISPATCH_LEVEL)
   {
       return;
   }

   liInterval.QuadPart = -1 * (LONGLONG)50;

   while (TRUE)
   {
       BOOLEAN bBreak = TRUE;
       BOOLEAN bLockedResource = FALSE;//当前线程是否具有独占访问给定的资源
       BOOLEAN bLockedPagingIoResource = FALSE;
       bNeedReleaseResource = FALSE;
       bNeedReleasePagingIoResource = FALSE;

	   // 到fcb中去拿锁。
	   //分页IO资源
       if (pFcb->PagingIoResource)    //这个应该是缓冲区,缓冲是否不为NULL
		   //返回当前线程是否具有独占访问给定的资源。
           bLockedPagingIoResource = ExIsResourceAcquiredExclusiveLite(pFcb->PagingIoResource);

	   //查看元PageingIoResource缓冲后，再查看Resource缓冲， 总之一定要拿到这个锁。
       if (pFcb->Resource)//存储，或许这个就是缓冲指针
       {
           bLockedResource = TRUE; 
		   //对pFcb->Resource 是否有独占访问权限
           if (ExIsResourceAcquiredExclusiveLite(pFcb->Resource) == FALSE)
           {
			   //如果当前线程对资源变量不具有独占访问的权限
               bNeedReleaseResource = TRUE;//必须释放资源
               if (bLockedPagingIoResource)//如果分页资源是锁定的
               {
				   //ExAcquireResourceExclusiveLite是当前线程获得独占访问指定资源的权限，
				   //指定资源不能立即获取时的常规行为。
				   //如果TRUE，调用方被放入等待状态，直到资源可以获得。
				   //如果FALSE，该例程立即返回，无论资源是否可以获得。
                   if (ExAcquireResourceExclusiveLite(pFcb->Resource, FALSE) == FALSE)
                   {
                       bBreak = FALSE;//因为这里是死循环，失败的话，不跳出循环，下一次继续咨询权限
                       bNeedReleaseResource = FALSE;//没有取得独占权限（这里称之为锁），所以也不需要释放标志
                       bLockedResource = FALSE; //资源锁定的标志为FALSE
                   }
               }
               else//如果分页资源未锁定
				   //请求获取独占访问权限，直到成功为止
                   ExAcquireResourceExclusiveLite(pFcb->Resource, TRUE);
           }
       }
   
	   //如果分页资源未锁定
       if (bLockedPagingIoResource == FALSE)
       {
           if (pFcb->PagingIoResource)
           {
               bLockedPagingIoResource = TRUE;
               bNeedReleasePagingIoResource = TRUE;

			   //如果资源是锁定的，那么就用ExAcquireResourceExclusiveLite进行尝试性获取权限
			   //如果是是未锁定的，就一直等到独占成功为止
               if (bLockedResource)
               {
                   if (ExAcquireResourceExclusiveLite(pFcb->PagingIoResource, FALSE) == FALSE)
                   {
                       bBreak = FALSE;
                       bLockedPagingIoResource = FALSE;
                       bNeedReleasePagingIoResource = FALSE;//没获取成功，就不用释放资源了
                   }
               }
               else
               {
                   ExAcquireResourceExclusiveLite(pFcb->PagingIoResource, TRUE);
               }
           }
       }

       if (bBreak)//如果两个锁都拿到手了，跳出循环
       {
           break;
       }
       //下面是因为只拿到了零个或一个锁，所以要释放锁后重新获取
       if (bNeedReleasePagingIoResource)//如果资源之前独占成功了，那么这里就要释放
       {
           ExReleaseResourceLite(pFcb->PagingIoResource);
       }
       if (bNeedReleaseResource)//如果资源之前独占成功了，那么这里就要释放
       {
           ExReleaseResourceLite(pFcb->Resource);
       }

	   //如果是PASSIVE_LEVEL级别，则用KeDelayExecutionThread延迟
	   //否则用event延迟
       if (irql == PASSIVE_LEVEL)
       {
		   
           KeDelayExecutionThread(KernelMode, FALSE, &liInterval);
       }
       else
       {
           KEVENT waitEvent;
           KeInitializeEvent(&waitEvent, NotificationEvent, FALSE);
           KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, &liInterval);
       }
   }

   //PBCB Bcb = pFileObject->SectionObjectPointer->SharedCacheMap;
   //PMM_SECTION_SEGMENT Segment = (PMM_SECTION_SEGMENT)pFileObject->SectionObjectPointer->DataSectionObject;
   //PMM_IMAGE_SECTION_OBJECT ImageSectionObject = (PMM_IMAGE_SECTION_OBJECT)pFileObject->SectionObjectPointer->ImageSectionObject;
   //
   //如果系统中已经存在了该文件的文件缓冲，那么应用层的读写请求往往会被转化为Fast IO请求。
   //指向文件对象的只读节对象的指针。此成员仅由文件系统设置，用于高速缓存管理器交互。
   if (pFileObject->SectionObjectPointer)
   {
		IO_STATUS_BLOCK ioStatus;
		CcFlushCache(pFileObject->SectionObjectPointer, NULL, 0, &ioStatus);//清除缓存
		//清除缓存会将缓存文件的全部或部分刷新到磁盘。
		if (pFileObject->SectionObjectPointer->ImageSectionObject)
		{
			//如果镜像部分不为空，则刷新镜像段
			//MmFlushForWrite正在打开镜像段，以提供访问
			//清除镜像到内存
			MmFlushImageSection(pFileObject->SectionObjectPointer,MmFlushForWrite); // MmFlushForDelete
		}
		//從系統緩存中清除緩存文件的全部或部分
		CcPurgeCacheSection(pFileObject->SectionObjectPointer, NULL, 0, FALSE);
   }

   if (bNeedReleasePagingIoResource)//如果资源之前独占成功了，那么这里就要释放
   {
       ExReleaseResourceLite(pFcb->PagingIoResource);
   }
   if (bNeedReleaseResource)//如果资源之前独占成功了，那么这里就要释放
   {
       ExReleaseResourceLite(pFcb->Resource);
   }
}

