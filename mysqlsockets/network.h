#ifndef __NETWORK_H
#define __NETWORK_H
#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h>  
#include <arpa/inet.h> 

typedef struct sockaddr_in sockaddr_in_t;

typedef struct sockaddr sockaddr_t;

int createTcpSocket(void);

int initListenSocket(const char *ip,int port);

int setSocketNonBlocking(int fd);

int connectnew(sockaddr_in_t &addr);
/*
SO_REUSEADDR
SO_KEEPALIVE
SO_SNDBUF
SO_RCVBUF
*/
int setSocketOpt(int sockfd,int options,int value);

int senddata(int fd,const char*sendbuf,int len);
int recvdata(int fd,char*recvbuf,int len);

#ifdef __cplusplus
#include <iostream>
#include <string>
#include "CBuffer.h"
#include <json/json.h>
class CCliSocket
{
public:
	CCliSocket();
	
	~CCliSocket();
	
	bool connect(const char *ip,int port);

	bool isBreaking();
	
	void close();

	int getSockfd();
	
	int recv(char*recvbuf,int len);
	
	int send(char*sendbuf,int len);

private:

	int sockfd;
	
	CCliSocket&operator=(const CCliSocket&);

	CCliSocket(const CCliSocket&);
};

class CServSocket
{
public:
	CServSocket();
	
	~CServSocket();

	bool openListener(const std::string &ip,const int& port);
	
	bool acceptSocket(int &connfd,sockaddr_in_t &cliAddr);

	void close(void);

	int getSockfd(void);
	
	int recv(char *recvbuf,int len);

	int send(char *sendbuf,int len);
	
private:

	int listenSocket;

	CServSocket(const CServSocket&);
	
	CServSocket&operator=(const CServSocket&);
};
#endif
#endif
