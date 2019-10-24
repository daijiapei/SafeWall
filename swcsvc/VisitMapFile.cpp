
#include "swcsvc.h"
#include "..\\include\\assist.h"

BOOL OpenMapFile(OUT HANDLE * hMap,OUT LPMYSERVCONTEXT * lpContext,IN char * lpcMapName)
{
	*hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, 0, lpcMapName);
	if(NULL == *hMap)
	{
		return FALSE;
	}
	*lpContext = (LPMYSERVCONTEXT)MapViewOfFile(*hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if(NULL == *lpContext)
	{
		return FALSE;
	}
	return TRUE;
}

void ReleaseMapFile(IN HANDLE hMap,IN LPMYSERVCONTEXT lpContext)
{
	UnmapViewOfFile(lpContext);
	CloseHandle(hMap);
}
