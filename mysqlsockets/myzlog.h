#ifndef __MY_ZLOG__
#define __MY_ZLOG__
#include<zlog.h>
typedef zlog_category_t zlog_cat_t;

#define DEBUG ZLOG_LEVEL_DEBUG

#define INFO ZLOG_LEVEL_INFO

#define NOTICE ZLOG_LEVEL_NOTICE

#define WARN ZLOG_LEVEL_WARN

#define ERROR ZLOG_LEVEL_ERROR

#define FATAL ZLOG_LEVEL_FATAL

extern zlog_category_t *log_category;

void log_init(void);

void log_fini(void);

#define LOG(lev,...) zlog(log_category, __FILE__,\
	sizeof(__FILE__)-1,__func__, sizeof(__func__)-1, __LINE__, lev, __VA_ARGS__)

void vLog(const char*fmt,...);
//#define __DEBUG__
extern int stdoutValue;
#define LogMsgOut(...) vLog(__VA_ARGS__)
#endif
