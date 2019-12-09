#ifndef __CLIENT__H
#define __CLIENT__H
#include <string>
#include <iostream>
#include "network.h"
#include "CBuffer.h"
#include <json/json.h>
const int DEFAULT_CLIENT_NUMS = 1024;
class Client
{
public:
	Client();
	Client(const int&bufsize);
	int fd;
	std::string idnum;
	sockaddr_in_t addr;
	CBuffer cbuf;
	Client&operator=(const Client &);
	bool operator==(const Client & obj);
	Client(const Client&);
	void init();
private:
};


class ManageClients
{
public:

	friend class SSFX;
	
	ManageClients(const int &clientnums=1024);

	~ManageClients();

	/*
	*time:20170709
	*author:yl
	*version:1.0
	*ReadClient:读取客户端数据
	*/
	bool ReadClient(Client &client);

	bool ReadClient(int index);


	/*
	*time:20170709
	*author:yl
	*version:1.0
	*AddClient:向客户端集合添加一个客户端
	*/
	bool AddClient(Client&);

	/*
	*time:20170709
	*author:yl
	*version:1.0
	*DelClient:从客户端集合中删除一个客户端
	*/
	bool DelClient(Client&);

	bool DelClient(int index);

	/*
	*time:20170709
	*author:yl
	*version:1.0
	*GetClientIndex:获取客户端在集合中的索引
	*/
	int GetClientIndex(int fd);

	/*
	*time:20170709
	*author:yl
	*version:1.0
	*GetClient:获获取客户端引用
	*/
	Client &GetClient(int fd);

	int sendToClient(std::string idnums,const Json::Value &root) const ;
	
private:

	/*客户端集合*/
	Client *clients;

	/*客户端总数量*/
	int clientsum;

	/*拷贝构造*/
	ManageClients(const ManageClients&);

	/*赋值运算符*/
	ManageClients&operator=(const ManageClients&);	

	/*
	*time:20170709
	*author:yl
	*version:1.0
	*isclientlogin:判断是不是客户端注册数据
	*@parma:root,输入的json数据，client客户端
	*/

	bool IsClientLogin(Json::Value &root,Client&client);

	/*
	*time:20170709
	*author:yl
	*version:1.0
	*existedlogin:重复登陆处理
	*/
	bool existedLogin(const std::string&idnum,Client &client);
	
	/*
	*time:20170709
	*author:yl
	*version:1.0
	*client_buffer_handle:客户端缓存处理
	*/
	bool client_buffer_handle(Client &client);

		
	/*
	*time:20170709
	*author:yl
	*version:1.0
	*data_source_dispatch:按照不数据源不同，分配不同的功能
	*/
	bool data_source_dispatch(Json::Value &val,Client &);

	/*
	*time:20170709
	*author:yl
	*version:1.0
	*extractJson:提取json数据
	*/
	bool extractJson(const char *data,Client &client);

	/*
	*time:20170709
	*author:yl
	*version:1.0
	*extractJson:string数据转换成json
	*/
	bool stringToJson(const char*str,Json::Value &val);

};
extern int sendJson(int fd,Json::Value root);
#endif