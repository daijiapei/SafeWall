

#ifndef _MEMPOOL_H
#define _MEMPOOL_H

#include <stdlib.h>

#define POOLSIZE   200

class MemPool
{
private:
	int * poolFlag;
	void * pHeader;
	void * pEnder;
	int buflen;
	int poolsize;
	int count;
	

public:
	MemPool(int bufLen = 4096, int poolSize = 200);
	~MemPool();

	int getBuflen();
	int init();
	void bzero(void * dst, int size);
	void * alloc();
	void release(void *p);
};

inline MemPool::MemPool(int bufLen = 4096,int poolSize = 200)
{
	buflen = bufLen;
	poolsize = poolSize;

	poolFlag = 0;
	pHeader = 0;
}

inline int MemPool::getBuflen()
{
	return buflen;
}

inline MemPool::~MemPool()
{
	if(pHeader) free(pHeader);
	if(poolsize) free(poolFlag);
}

inline void * MemPool::alloc()
{
	if(count == poolsize)
	{
		return malloc(buflen);
	}

	for(int i=0; i < poolsize; i++)
	{
		if(!poolFlag[i])
		{
			poolFlag[i] = 1;
			++count;
			return (void *)((int)pHeader + (buflen * i));
		}
	}
}

inline void MemPool::release(void *p)
{
	if(!p) return;
	if(p > pEnder || p < pHeader) 
	{
		free(p);
		return;
	}
	poolFlag[((int)p - (int)pHeader) / buflen] = 0;
	--count;
}

inline int MemPool::init()
{
	
	if(!buflen|| !poolsize) return 0;
	if(pHeader) return 1;
	poolFlag = (int *) malloc(sizeof(int) * poolsize);
	if(!poolFlag) return 0;

	pHeader = malloc(buflen * poolsize);
	if(!pHeader) 
	{
		free(poolFlag);
		return 0 ;
	}
	pEnder = (void*)((int)pHeader + (buflen * poolsize));
	bzero(poolFlag, sizeof(int) * poolsize);
	bzero(pHeader, buflen * poolsize);
	return 1;
}

inline void MemPool::bzero(void * dst, int size)
{
	char * p = (char *)dst;
	while (size--)  *p++ = 0;
}

#endif