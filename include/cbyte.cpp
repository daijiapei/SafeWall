
#include "cbyte.h"

Cbyte::Cbyte()
{
	m_psrc = 0;
	m_pformat = 0;
	m_src_len = 0;
}

Cbyte::Cbyte(char * format)
{

}

Cbyte::Cbyte(char * src,int len)
{
}

int Cbyte::append(char * format)
{
}

int Cbyte::append(char * src, int len)
{
}

int Cbyte::srclen()
{
	return m_src_len;
}

char* Cbyte::psrc() const
{
	return m_psrc;
}

char * Cbyte::pformat() const
{
}