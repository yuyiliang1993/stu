#include "mutexLock.h"#include <iostream>pthread_mutex_t mutex;
MyCMutex::MyCMutex(){	std::cout<<"MyCMutex()"<<std::endl;
	pthread_mutex_init(&mutex,NULL);
}
	
MyCMutex::~MyCMutex(){	std::cout<<"~MyCMutex()"<<std::endl;
	pthread_mutex_destroy(&mutex);
}


void MyCMutex::lock(void){
	pthread_mutex_lock(&mutex);
}

void MyCMutex::unlock(void){
	pthread_mutex_unlock(&mutex);
}int init_mutex(pthread_mutex_t *mutex){	pthread_mutex_init(mutex,NULL);}int destory_mutex(pthread_mutex_t *mutex){	pthread_mutex_destroy(mutex);}int lock_mutex(pthread_mutex_t *mutex){	pthread_mutex_lock(mutex);}int unlock_mutex(pthread_mutex_t *mutex){	pthread_mutex_unlock(mutex);}

