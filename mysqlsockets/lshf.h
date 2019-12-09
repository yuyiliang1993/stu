#ifndef __LSHF_H__
#define __LSHF_H__
#include <mysql/mysql.h>
#include <json/json.h>
#include "querydb.h"

#include <iostream>
#include <mutex>
#if 0
#ifdef _cplusplus
extern "C"{
#endif
struct LshfInfo{
	MYSQL_RES * res;
	int socketid;
};
#ifdef _cplusplus
}
#endif
#endif
class LshfInfo
{
/*
*time:20170719
*author:yl
*function:回放需要的数据结构
*/

public:
	LshfInfo():res(NULL),socketid(-1),sumSendBytes(0UL),requestSock(-1){
	}
	void init(void);
	
/*结果集*/
	MYSQL_RES *res;
	
/*客户端sockid*/
	int socketid;

/*记录已发送的所有字节数*/
	unsigned long sumSendBytes;

/*请求套接字*/
	int requestSock;

private:
	
/*拷贝构造*/	
	LshfInfo(const LshfInfo &obj);

/*赋值运算*/
	LshfInfo&operator=(const LshfInfo &obj);	
};


class ReqArrData
{
public:
	ReqArrData(int nums = 1024):maxArrayNums(nums){
	//	std::cout<<"ReqArrData"<<std::endl;
		LshfInfoRequestArrays = new LshfInfo[nums];
	//	pthread_lshf_value = 0;
	}
	~ReqArrData(){
		delete[]LshfInfoRequestArrays;
	}
	
	/*历史回放组*/
		LshfInfo *LshfInfoRequestArrays;//请求组
	
	/*回放组数量*/
		const int maxArrayNums;

	//	int pthread_lshf_value;

	/**/
private:
	
	ReqArrData(const ReqArrData &);

	ReqArrData&operator=(const ReqArrData &);
};


class LSHF:public MySqlCAPI
{
public:
	static LSHF *getInstance(void);

/*开始回放*/
	bool start(MYSQL &sqlfd,Json::Value root,int sock);

/*停止回放*/
	bool stop(Json::Value root);

	void destoryInst();

	static ReqArrData requestArraydata;
	
private:

/*加入一个查询结果到数组*/	
	void insertArrays(MYSQL_RES *res,int socketid,int requestSocket);

/*判断线程是否存活*/
	bool isPthreadAlive(void);

/*拷贝构造*/
	LSHF(const LSHF&);

/*赋值运算符*/	
	LSHF&operator=(LSHF&);

	LSHF(){}
	
	~LSHF(){}
	
/*	class FR{
		public:
		~ FR(){
			if(lshf != NULL){
				delete lshf;
				lshf = NULL;
			}
		}
		static FR fr;
	}*/
	static LSHF * lshf;
	static std::mutex m_mutex;
	/*保存线程的id号*/
	static pthread_t  pid;
};

#endif
