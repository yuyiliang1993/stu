#include "CEpoll.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>

using namespace std;

CEpoll::CEpoll(const int &eventnums)
{
	maxEventNums = eventnums;
	epfd = epoll_create(maxEventNums + 1);
	if(epfd < 0){
		perror("epoll_create");
		exit(1);
	}
	
	try{
		activeEvents = new epoll_event[maxEventNums+1];
	}catch(std::bad_alloc & e){
		cerr<<"new epoll_event error:"<<e.what()<<endl;
		close(epfd);
		exit(1);
		activeEvents = 0;
	}
	et = false;
}

CEpoll::CEpoll(const CEpoll &obj)
{
	maxEventNums = obj.maxEventNums;
	epfd = epoll_create(maxEventNums + 1);
	if(epfd < 0){
		perror("epoll_create");
		exit(1);
	}
	activeEvents = new epoll_event[maxEventNums+1];
	et = false;
}

CEpoll& CEpoll::operator=(const CEpoll&obj)
{
	if(&obj == this)
		return *this;

	maxEventNums = obj.maxEventNums;
	if(epfd>=0)
		close(epfd);
	epfd = epoll_create(maxEventNums + 1);
	if(epfd < 0){
		perror("epoll_create");
		exit(1);
	}
	if(activeEvents)
		delete[]activeEvents;
	activeEvents = new epoll_event[maxEventNums+1];
	et=false;
	return *this;
}

CEpoll::~CEpoll()
{
	if(epfd >=0 ){
		close(epfd);
		epfd = -1;
	}
	
	if(activeEvents){
		delete[]activeEvents;
		activeEvents = 0;
	}
}

void CEpoll::setEt(bool & t)
{
	et = t;
}

bool CEpoll::event_add(const int&fd,const short&event)
{
	ev.events = et?event|EPOLLET:event;
	ev.data.fd = fd;
	return (0==epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev));
}

bool CEpoll::event_del(const int&fd,const short&event)
{
	ev.events = event;
	ev.data.fd = fd;
	return (0 == epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&ev));
}

bool CEpoll::event_mod(const int&fd,const short&event)
{
	ev.events = event;
	ev.data.fd = fd;
	return (0==epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev));
}

int CEpoll::event_wait(const int & msec)
{
	return epoll_wait(epfd,activeEvents,maxEventNums,msec);
}

epoll_event_t *CEpoll::getActiveEvents(void)
{
	return activeEvents;
}