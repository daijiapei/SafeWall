
#ifndef _class_byte_h
#define _class_byte_h

class Cbyte
{
public:
	Cbyte();
	Cbyte(char * src, int len);
	Cbyte(char * format);
	~Cbyte();

	
	int append(char * src, int len);
	int append(char * format);

	int srclen();
	char * psrc() const;
	char * pformat() const;

private:
	char * m_psrc;
	char * m_pformat;
	int m_src_len;

};

#endif