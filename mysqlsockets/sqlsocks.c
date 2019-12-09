#include "serverwork.h"
#include "myzlog.h"
#include "client.h"
#include "querydb.h"
#include "mutexLock.h"
int main(int argc,char *argv[])
{
	signal(SIGPIPE,SIG_IGN);
	init_mutex(&mutex);
	log_init();
	ServerWork sWork(5);
	sWork.loopStart();
	log_fini();
	destory_mutex(&mutex);
}
