#ifndef __QUERY_DB_H
#define __QUERY_DB_H
#include <mysql/mysql.h>
#include <json/json.h>
class MySqlCAPI
{
public:
	
/**
*time:20180716
*author:yl
*version:1.0
*query:请求执行sql语句
*@param-sqlfd:mysql数据库句柄
*@param-sqlbuffer:操作数据库语句
*@param-len:语句的长度
*return:反馈执行语句的成功和失败
*/
	bool query(MYSQL &sqlfd,const char*sqlbuffer,int len);

/**
*time:20180716
*author:yl
*version:1.0
*queryRows:获取查询的行数
*@param-sqlfd:mysql数据库句柄
*@param-sqlbuffer:操作数据库语句
*@param-len:语句的长度
*return:反馈执行语句的成功和失败
*/
	long queryRowNum(MYSQL &sqlfd,const char*sqlbuffer,int len);

/**
*time:20180716
*author:yl
*version:1.0
*queryResult:请求结果集
*@param-sqlfd:mysql数据库句柄
*@param-sqlbuffer:操作数据库语句
*@param-len:语句的长度
*return:反馈指向结果集合的指针，等于NULL表示获取失败,结果集使用完需要释放
*/	
	MYSQL_RES *queryResult(MYSQL &sqlfd,const char*sqlbuffer,int len);

/**
*time:20180716
*author:yl
*version:1.0
*fetchRow:从结果集合中获取一行
*@param-result:不为空值的结果集
*return:反馈查询的一行
*/	
	MYSQL_ROW fetchRow(MYSQL_RES *result);

/**
*time:20180716
*author:yl
*version:1.0
*getFields:获取结果集的列数
*@param-result:不为空值的结果集
*return:反馈查询的一行
*/	
	int getFields(MYSQL_RES *result);

/**
*time:20180716
*author:yl
*version:1.0
*getRowNums:获取结果集的总行数
*@param-result:不为空值的结果集
*return:反馈结果集的总行数
*/	
	int getRowNums(MYSQL_RES *result);

/**
*time:20180716
*author:yl
*version:1.0
*freeResult:释放结果集
*@param-result:被释放的结果集合
*/	
	void freeResult(MYSQL_RES *result);

private:
//	MySqlCAPI(const MySqlCAPI&);
//	MySqlCAPI&operator=(const MySqlCAPI&);
};

class User{
public:
private:
};
#endif