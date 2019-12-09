#include "CBuffer.h"
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
#include <iostream>
CBuffer::CBuffer(const uint_t &size)
{
	totalLens = size;
	newBuffer(totalLens);
	offetLens = 0;
}

CBuffer::~CBuffer(void)
{
	if(buffer){
		delete[]buffer;
	//	std::cout<<"~cbuffer()"<<std::endl;
	}
}

CBuffer::CBuffer(const CBuffer&obj)
{
	totalLens = obj.totalLens;
	offetLens = obj.offetLens;
	this->newBuffer(totalLens);
}

CBuffer & CBuffer::operator=(const CBuffer&obj)
{
	if(this == &obj)
		return *this;
	if(buffer)
		delete[]buffer;
	totalLens = obj.totalLens;
	newBuffer(totalLens);
	offetLens = obj.offetLens;
	return *this;
}

void CBuffer::newBuffer(const uint_t &size)
{
	try{
		buffer = new char[size];
	}catch(std::bad_alloc &e){
		std::cerr<<"new Failed:"<<e.what()<<std::endl;
		buffer = NULL;
		exit(0);
	}
	memset(buffer,0,size);
}
#endif
