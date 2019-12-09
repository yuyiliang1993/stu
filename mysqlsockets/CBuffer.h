#ifndef __CBUFFER_H
#define __CBUFFER_H
const int  DEFAULT_BUFFER_SIZE = 1024*10;//默认大小是10k 
typedef unsigned int uint_t;

#ifdef __cplusplus

class CBuffer
{
/*
*@time:2018.6.3
*@author:yuliang
*buffer:动态内存
*offetlens:buffer被填充的偏移
*totallens:buffer总长度
*最大长度为2**31
*/
public:
	
	CBuffer(const uint_t &size = DEFAULT_BUFFER_SIZE);
	
	
	virtual ~CBuffer(void);
	
	CBuffer & operator = (const CBuffer&);

	CBuffer(const CBuffer&);

	char*buffer;

	int offetLens;

	int totalLens;
	
private:
	/*
	*new内存，并捕捉new异常
	*/
	void newBuffer(const uint_t &size);

};
#endif
#endif
