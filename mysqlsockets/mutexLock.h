#ifndef __MUTEX_LOCK_H
#define __MUTEX_LOCK_H
#include <pthread.h>
class MyCMutex
{
public:
	
/**
*time:20180716
*author:yl
*version:1.0
*CMutex:创建一个mutex句柄
*/	
	MyCMutex();
/**
*time:20180716
*author:yl
*version:1.0
*~CMutex:销毁句柄
*/		
	~MyCMutex();

/**
*time:20180716
*author:yl
*version:1.0
*mutexLock:加锁
*/	
	void lock(void);

/**
*time:20180716
*author:yl
*version:1.0
*mutexLock:解锁
*/	
	void unlock(void);

/**
*time:20180716
*author:yl
*version:1.0
*mutex:线程锁
*/
	pthread_mutex_t mutex;
private:
	
/*拷贝构造*/

	MyCMutex(const MyCMutex&);

/*赋值运算符重载*/
	MyCMutex&operator=(const MyCMutex&);
};

int init_mutex(pthread_mutex_t *mutex);

int destory_mutex(pthread_mutex_t *mutex);

int lock_mutex(pthread_mutex_t *mutex);

int unlock_mutex(pthread_mutex_t *mutex);
extern pthread_mutex_t mutex;
#endif