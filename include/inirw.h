/**
 * 文件：inirw.h
 * 版本：1.0
 * 作者：taoyuanmin@gmail.com
 *
 * 说明：ini配置文件读写
 * 1、支持;和#注释符号，支持行尾注释。
 * 2、支持带引号'或"成对匹配的字符串，提取时自动去引号。引号中可带其它引号或;#注释符。
 * 3、支持无section或空section(名称为空)。
 * 4、支持10、16、8进制数，0x开头为16进制数，0开头为8进制。
 * 5、支持section、key或=号前后带空格。
 * 6、支持\n、\r、\r\n或\n\r换行格式。
 * 7、不区分section、key大小写，但写入时以新串为准，并保持其大小写。
 * 8、新增数据时，若section存在则在该节最后一个有效数据后添加，否则在文件尾部添加。
 * 9、支持指定key所在整行删除，即删除该键值，包括注释。
 * 10、可自动跳过格式错误行，修改时仍然保留。
 * 11、修改时保留原注释：包括整行注释、行尾注释(包括前面空格)。
 * 12、修改时保留原空行。以上三点主要是尽量保留原格式。
 */
 

#ifndef _INI_RW_H_
#define _INI_RW_H_

#define SIZE_LINE		1024	//每行最大长度
#define SIZE_FILENAME	256		//文件名最大长度

#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

//#define min(x, y)		(x <= y) ? x : y

#define __INIOBJECT  int*
#define INIOBJECT  __INIOBJECT

typedef enum _ELineType_ {
    LINE_IDLE,		//未处理行
	LINE_ERROR,		//错误行
	LINE_EMPTY,		//空白行或注释行
	LINE_SECTION,	//节定义行
	LINE_VALUE		//值定义行
} ELineType ;


typedef struct _INI_OBJENT {
	char *buffer;
	int  buflen;
	char fileName[SIZE_FILENAME];
}* LPINIOBJECT;

#ifdef __cplusplus
extern "C" {
#endif

//创建ini对象
INIOBJECT CreateIniObject(const char * fileName);
//释放INI对象
void ReleaseIniObject(INIOBJECT iniobj);
//加载ini文件至内存
int iniFileLoad(INIOBJECT iniobj);
//获取字符串，不带引号
int iniGetString(INIOBJECT iniobj, const char *section, const char *key, char *value, int maxlen, const char *defvalue);
//获取整数值
int iniGetInt(INIOBJECT iniobj, const char *section, const char *key, int defvalue);
//获取浮点数
double iniGetDouble(INIOBJECT iniobj, const char *section, const char *key, double defvalue);

//设置字符串：若value为NULL，则删除该key所在行，包括注释
int iniSetString(INIOBJECT iniobj, const char *section, const char *key, const char *value);
//设置整数值：base取值10、16、8，分别表示10、16、8进制，缺省为10进制
int iniSetInt(INIOBJECT iniobj, const char *section, const char *key, int value, int base);

#ifdef __cplusplus
}
#endif

#endif
