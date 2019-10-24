

#include "swssvc.h"

//实际上，本函数是不允许失败的
BOOL ReadPolicy(FILEEVENT * file)
{
	Json::Reader reader;
	FILE * pfile = {0};
	BOOL ret = FALSE;
	EnterCriticalSection(&file->section);
	
	fopen_s(&pfile, file->FilePath, "rb");

	if(pfile != NULL)
	{
		fseek(pfile, 0, SEEK_END);
		long len = ftell(pfile);
		char * buffer = (char *)_HeapAlloc(len);
		if(buffer)
		{
			fseek(pfile,0,SEEK_SET);
			fread(buffer, 1, len, pfile);
			fclose(pfile);

			if(true == reader.parse(buffer,*file->root))
			{
				ret = TRUE;
			}
			_HeapFree(buffer);
		}
	}
	LeaveCriticalSection(&file->section);

	return ret;
}

//实际上，本函数是不允许失败的
BOOL SavePolicy(FILEEVENT * file)
{
	FILE *pfile = {0};
	BOOL ret = FALSE;
	EnterCriticalSection(&file->section);
	fopen_s(&pfile, file->FilePath, "wb");
	if(pfile != NULL)
	{
		std::string str = file->root->toStyledString();

		if(fwrite(str.c_str(), 1,  str.length(), pfile))
		{
			ret = TRUE;
		}
		else
		{
			
		}
		fclose(pfile);
	}

	LeaveCriticalSection(&file->section);
	return ret;
}
