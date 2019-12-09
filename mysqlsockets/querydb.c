#include "querydb.h"
#include "myzlog.h"
bool  MySqlCAPI::query(MYSQL &sqlfd,const char*sqlbuffer,int len)
{
	if(mysql_real_query(&sqlfd,sqlbuffer,len)){
		LOG(ERROR,"query err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		return false;
	}
	return true;
}


long MySqlCAPI::queryRowNum(MYSQL &sqlfd,const char*sqlbuffer,int len)
{
	if(!query(sqlfd,sqlbuffer,len)){
		return 0;
	}

	MYSQL_RES *res = mysql_store_result(&sqlfd);
	if(res == NULL){
		return 0;
	}
	
	long nums = mysql_num_rows(res);
	mysql_free_result(res);
	return nums;
}


MYSQL_RES *MySqlCAPI::queryResult(MYSQL &sqlfd,const char*sqlbuffer,int len)
{
	if(!query(sqlfd,sqlbuffer,len))
		return NULL;
#if 0
	MYSQL_RES * res = mysql_store_result(&sqlfd);
	if(res == NULL){
		LOG(ERROR, "msyql_store_result err:%s",mysql_error(&sqlfd));
	}
#endif
	return (::mysql_store_result(&sqlfd));	
}

MYSQL_ROW MySqlCAPI::fetchRow(MYSQL_RES *result)
{
	return mysql_fetch_row(result);	
}

int MySqlCAPI::getFields(MYSQL_RES *result)
{
	return mysql_num_fields(result);		
}

int MySqlCAPI::getRowNums(MYSQL_RES *result)
{
	return mysql_num_rows(result);
}

void MySqlCAPI::freeResult(MYSQL_RES *result)
{
	if(result)
		mysql_free_result(result);
}