#include "mutexLock.h"
MyCMutex::MyCMutex(){
	pthread_mutex_init(&mutex,NULL);
}
	
MyCMutex::~MyCMutex(){
	pthread_mutex_destroy(&mutex);
}


void MyCMutex::lock(void){
	pthread_mutex_lock(&mutex);
}

void MyCMutex::unlock(void){
	pthread_mutex_unlock(&mutex);
}

