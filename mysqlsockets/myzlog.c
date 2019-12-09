#include "myzlog.h"
#include <stdio.h>
#include <stdlib.h>
zlog_category_t *log_category;
int stdoutValue = 0;
void log_fini(void)
{
	zlog_fini();
}

void log_init(void)
{
	if(zlog_init("zlog.conf")){
		printf("Error:zlog_init\n");
		zlog_fini();
		exit(1);
	}
	log_category = zlog_get_category("system");
	if(!log_category){
		printf("Error:zlog_get_category\n");
		zlog_fini();
		exit(1);
	}
}

void vLog(const char*fmt,...)
{
	char buffer[4096];
	va_list arg;
	va_start(arg,fmt);
	vsnprintf(buffer,sizeof(buffer),fmt,arg);
//	printf("%s",buffer);
//	fprintf(stdout,"%s",buffer);
	fputs(buffer,stdout);
	va_end(arg);
}