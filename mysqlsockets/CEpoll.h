#ifndef __CEPOLL_H
#define __CEPOLL_H
#include <sys/epoll.h>
typedef struct epoll_event epoll_event_t;

#define EV_READ EPOLLIN

#define EV_WRITE EPOLLOUT

class CEpoll
{
/*
*time:20180608
*auth:yl
*funtion:监听文件描述符的事件，epoll机制
*默认的工作模式为水平触发
*默认监听的文件描述符是1024
*/
public:
/*
*默认参数构造函数
*/	
	CEpoll(const int &eventnums = 32000);
	
	virtual ~CEpoll(void);
/*
*添加文件描述符的事件
*/	
	bool event_add(const int&,const short&);
/*
*删除文件描述符的事件
*/	
	bool event_del(const int&,const short&);
/*
*修改文件描述符的事件
*/
	bool event_mod(const int&,const short&);
/*
*返回大于0表示活跃的文件描述符数量，发生的事件通过getActiveEvents()获得
*返回小于0表示监听出错
*返回等于0表示监听超时
*/	
	int event_wait(const int & msec);
/*
*设置工作模式;true表示边缘触发，false表示水平触发
*/
	void setEt(bool &);
/*
*获取活跃的事件
*/	
	epoll_event_t *getActiveEvents();
	
private:
	int epfd;
	int maxEventNums;
	bool et;
	epoll_event_t ev;
	epoll_event_t *activeEvents;
/*
*深拷贝
*/
	CEpoll(const CEpoll &);
/*
*赋值运算符
*/	
	CEpoll &operator=(const CEpoll &);
};
#endif
