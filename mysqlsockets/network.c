#include "network.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <iostream>
#include <errno.h>
#include "myzlog.h"
static int make_socket_reuseable(int fd)
{
	unsigned value = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value,sizeof(value)) < 0){
		return -1;
	}
	return 0;
}

static int  make_socket_nonblocking(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
		perror("fcntl");
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0){
		perror("fcntl");
		return -1;
	}
	return 0;
}

int createTcpSocket(void)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){
		perror("socket error!");
		exit(1);
	}
}

int setSocketNonBlocking(int fd)
{
	return make_socket_nonblocking(fd);
}

int initListenSocket(const char *ip,int port)
{
	int sock  = -1;	
	if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0){
		return -1;
	}
	setSocketOpt(sock,SO_REUSEADDR,1);
	setSocketOpt(sock,SO_KEEPALIVE,1);
	sockaddr_in_t addr;
	bzero(&addr,sizeof(sockaddr_in_t));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(NULL == ip)
		addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(ip);
	else
		addr.sin_addr.s_addr = inet_addr(ip);
	if(bind(sock,(sockaddr_t*)&addr,sizeof(addr))<0||listen(sock,10)<0){
		close(sock);
		return -1;
	}
	return sock;
}

int setSocketOpt(int sockfd,int options,int value)
{
	 return setsockopt(sockfd,SOL_SOCKET,options,(const void*)&value,sizeof(value));
}

int connectnew(sockaddr_in_t &addr)
{
	int sockfd ;
	if((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0){
		return (-1);
	}
	if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr)) < 0){
		close(sockfd);
		return (-1);
	}
	return sockfd;
}

int recvdata(int fd,char*recvbuf,int len)
{
	int ret = 0;
	do{
		ret =::recv(fd,recvbuf,len,0);
	}while(ret<0 &&(errno == EAGAIN||errno == EINTR));
	return ret;
}
	
int senddata(int fd,const char*sendbuf,int len)
{
	int ret = 0;
	do{
		ret =::send(fd,sendbuf,len,0);
	}while(ret<0 &&(errno == EAGAIN||errno == EINTR));
	return ret;
}

#ifdef __cplusplus

CCliSocket::CCliSocket(void)
{
	sockfd = -1;
}

CCliSocket::~CCliSocket(void)
{
	close();
}

void CCliSocket::close(void)
{
	if(sockfd >= 0){
		::close(sockfd);
		sockfd = -1;
	}
}

bool CCliSocket::connect(const char *ip,int port)
{
	if(ip == NULL || strlen(ip) <= 0 ||port <= 0){
		return false;
	}
	close();
	sockfd = createTcpSocket();
	printf("sockfd:%d\n",sockfd);
	sockaddr_in_t addr;
	bzero(&addr,sizeof(sockaddr_in_t));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	if(::connect(sockfd,(sockaddr_t*)&addr,sizeof(sockaddr_in_t))<0){
		close();
		return false;
	}
	return true;
}

bool CCliSocket::isBreaking(void)
{
	return sockfd<0? true:false;
}

int CCliSocket::getSockfd(void)
{
	return sockfd;		
}

int CCliSocket::recv(char*recvbuf,int len)
{
	int ret = 0;
	do{
		ret =::recv(sockfd,recvbuf,len,0);
	}while(ret<0 &&(errno == EAGAIN||errno == EINTR));
	return ret;
}

int CCliSocket::send(char*recvbuf,int len)
{
	int ret = 0;
	do{
		ret =::send(sockfd,recvbuf,len,0);
	}while(ret<0 &&(errno == EAGAIN||errno == EINTR));
	return ret;
}






/*****************************************************************
**/
CServSocket::CServSocket(void)
{
	listenSocket = -1;
}
	
CServSocket::~CServSocket(void)
{
	this->close();
}
	
bool CServSocket::openListener(const std::string &ip,const int &port)
{
	sockaddr_in_t addr;
	if((listenSocket = socket(AF_INET,SOCK_STREAM,0)) < 0){
		return false;
	}
	setSocketOpt(listenSocket,SO_REUSEADDR,1);//地址复用
	setSocketOpt(listenSocket,SO_KEEPALIVE,1);//保持长连接
	bzero(&addr,sizeof(sockaddr_in_t));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr=(ip.size()==0)?INADDR_ANY:inet_addr(ip.c_str());
	if(bind(listenSocket,(sockaddr_t*)&addr,sizeof(addr))<0||listen(listenSocket,10)<0){
		close();
		return false;
	}
	return true;	
}

int CServSocket::getSockfd(void){
	return listenSocket;
}

bool CServSocket::acceptSocket(int &connfd,sockaddr_in_t &cliAddr)
{
	socklen_t addrlen = sizeof(cliAddr);
	int ret = ::accept(listenSocket,(sockaddr_t*)&cliAddr,&addrlen);
	if(ret < 0)
		return false;
	connfd = ret;
	return true;
}	

void CServSocket::close(void)
{
	if(listenSocket >= 0){
		::close(listenSocket);
		listenSocket = -1;
	}
}
int CServSocket::recv(char*recvbuf,int len)
{
	int ret = 0;
	do{
		ret =::recv(listenSocket,recvbuf,len,0);
	}while(ret<0 &&(errno == EAGAIN||errno == EINTR));
	return ret;
}
	
int CServSocket::send(char*recvbuf,int len)
{
	int ret = 0;
	do{
		ret =::send(listenSocket,recvbuf,len,0);
	}while(ret<0 &&(errno == EAGAIN||errno == EINTR));
	return ret;
}
#endif
