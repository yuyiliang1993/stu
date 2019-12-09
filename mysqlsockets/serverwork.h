#ifndef __SERVERWORK_H
#define __SERVERWORK_H
#include "myzlog.h"
#include "client.h"
#include "CEpoll.h"
#include "network.h"
#include "apue.h"
#include <json/json.h>
#include "sqlhandle.h"
#include "client.h"

class ConnectSocket:public CCliSocket
{
public:
	ConnectSocket(int maxBufferLen = 1024*8);
	~ConnectSocket() = default;
	bool connect_event_read(const ManageClients&mclient);	
	bool buffer_handle(void);
	bool dataJsonDispose(Json ::Value &root);
	bool extractJsonData(const char *data);
	void loginToServer();
	void setLoginValue(bool value);
	bool isIdStatus(const char *dat,int len);
	CBuffer cbuffer;
private:
	bool initConnectMYSQL(void);
	SqlNet connSql;
	const ManageClients *mclient;
	bool loginValue;
	int localId;
};

class ServerWork
{
public:
/*
*数据监听
*数据读取
*数据解析
*数据反馈
*/
	ServerWork(int clientnums=1024);

	~ServerWork(void);
	
	void loopStart(void);
	
	void close(void);
	
private:
	
	ServerWork(const ServerWork&);
	
	ServerWork&operator=(const ServerWork&);

	void delfd(int fd,short event);
	
	void acceptClient(void);

	bool isListenSocket(int fd);

	bool isConnectSocket1(int fd);

	void disposeEvents(int eventnums);
	
	bool connect_socket1(std::string ip,int port);

	CEpoll *epfd;//32000

	CServSocket *sSocket;

	ManageClients *mClient;
	
	ConnectSocket connectSocket1;
};
#endif