#ifndef __SQLHANDLE_H
#define __SQLHANDLE_H
#include <mysql/mysql.h>
#include <json/json.h>
const int SQL_BUFFER_LEN =1024*1024;
class SqlNet
{
public:

	SqlNet(void);
	
	virtual ~SqlNet(void);

	bool Connect(const char *ip,const char *user,const char *passwd,const char *dbname);
	
	bool SetOptReconnect(void);

	bool IsConnectionBreak(void);
	
	MYSQL &GetFd(void);

	bool query(const char *sqlbuf,int sqllen);

	int queryRows(const char *sqlbuf,int sqllen);

	MYSQL_RES *queryRes(const char *sqlbuf,int sqllen);

	MYSQL_ROW getRow(MYSQL_RES *);

	int getFields(MYSQL_RES *);

	int getRowNums(MYSQL_RES *);
	
	void result_free(MYSQL_RES *);
	
	void close();

protected:	
	MYSQL sqlfd;
	bool connectionValue;
private:
	SqlNet &operator=(const SqlNet&);
	
	SqlNet(const SqlNet&);

};

class ManageClients;
class SqlWork :public SqlNet
{
public:
	
	SqlWork(int sql_buf_len = SQL_BUFFER_LEN);
		
	~SqlWork();
	
	bool query_analysis(Json::Value &val,int sock,const ManageClients &);
	
/*上行数据解码*/
	bool SXSJ(Json::Value&root,int sock);
/*用户*/
	bool YHZJ(Json::Value&root,int sock);
	bool YHSC(Json::Value&root,int sock);
	bool YHXG(Json::Value&root,int sock);
	bool YHCX(Json::Value&root,int sock);
/*公司*/
	bool GSZJ(Json::Value&root,int sock);
	bool GSSC(Json::Value&root,int sock);
	bool GSXG(Json::Value&root,int sock);
	bool GSCX(Json::Value&root,int sock);
/*航点*/
	bool HDZJ(Json::Value&root,int sock);
	bool HDSC(Json::Value&root,int sock);
	bool HDXG(Json::Value&root,int sock);
	bool HDCX(Json::Value&root,int sock);
/*航线*/
	bool HXZJ(Json::Value&root,int sock);
	bool HXSC(Json::Value&root,int sock);
	bool HXXG(Json::Value&root,int sock);
	bool HXCX(Json::Value&root,int sock);
/*空域*/
	bool KYZJ(Json::Value&root,int sock);
	bool KYSC(Json::Value&root,int sock);
	bool KYXG(Json::Value&root,int sock);
	bool KYCX(Json::Value&root,int sock);

/*飞机*/
	bool FJZJ(Json::Value&root,int sock);
	bool FJSC(Json::Value&root,int sock);
	bool FJXG(Json::Value&root,int sock);
	bool FJCX(Json::Value&root,int sock);

/*飞行员*/
	bool FXYZJ(Json::Value&root,int sock);
	bool FXYSC(Json::Value&root,int sock);
	bool FXYXG(Json::Value&root,int sock);
	bool FXYCX(Json::Value&root,int sock);
	
/*任务*/
	bool RWZJ(Json::Value&root,int sock);
	bool RWSC(Json::Value&root,int sock);
	bool RWXG(Json::Value&root,int sock);
	bool RWCX(Json::Value&root,int sock);

/*设备*/
	bool SBZJ(Json::Value&root,int sock);
	bool SBSC(Json::Value&root,int sock);
	bool SBXG(Json::Value&root,int sock);
	bool SBCX(Json::Value&root,int sock);

/*数据源*/
	bool SJYZJ(Json::Value&root,int sock);
	bool SJYSC(Json::Value&root,int sock);
	bool SJYXG(Json::Value&root,int sock);
	bool SJYCX(Json::Value&root,int sock);

/*设备类型*/
	bool SBLXZJ(Json::Value&root,int sock);
	bool SBLXSC(Json::Value&root,int sock);
	bool SBLXXG(Json::Value&root,int sock);
	bool SBLXCX(Json::Value&root,int sock);
	
/*密码查询*/
	bool MMCX(Json::Value&root,int sock);
/*日总飞行*/
	bool RZFX(Json::Value&root,int sock);
/*设备列表*/
	bool SBLB(Json::Value&root,int sock);
/*数备类型列表*/
	bool SBLXLB(Json::Value&root,int sock);
/*数据源列表*/
	bool SJYLB(Json::Value&root,int sock);
/*飞机列表*/
	bool FJLB(Json::Value&root,int sock);
/*飞行员列表*/
	bool FXYLB(Json::Value&root,int sock);
/*用户列表*/
	bool YHLB(Json::Value&root,int sock);
/*公司列表*/
	bool GSLB(Json::Value&root,int sock);

private:

	bool initMySql(void);

	bool create_all_table(void);
		
	bool checkjson(Json::Value &root);

	bool backfailure(int fd,Json::Value &root,const std::string &type,const std::string &reason);
	
	bool backsuccess(int fd,Json::Value &root,const std::string &type,const std::string &reason);

	bool query_handle(Json::Value &val,int sock);
	
	char * sqlbuffer;

	int max_sql_buffer_len;

	const ManageClients *mclient;
};

class RZ
{
	public:
		virtual bool Select(MYSQL &sqlfd,std::string tablename,Json::Value &root,int sock) = 0;
};

class PlaneRZ:public RZ
{
	public:
	bool Select(MYSQL &sqlfd,std::string tablename,Json::Value &root,int sock);
	private:
		/*
		*time:20180711
		*author:yl
		*version:1.0
		*selectTimeInfo:查询开始时间和结束时间
		*parma:被查询的飞机id
		*return:返回一个start和end的时间组
		*/
		Json::Value selectTimeInfo(int planeid);
		/*mysql句柄*/
		MYSQL sqlfd;
		/*查询的表名*/
		std::string dayTableName;
		Json::Value root;
		int sock;
		char sqlbuffer[2048];
};
class PilotRZ:public RZ
{
public:
	/*查询并反馈日表*/	
	bool Select(MYSQL &sqlfd,std::string tablename,Json::Value &root,int sock);
private:
	/*获取时间结构体*/
	Json::Value selectTimeInfo(int pilotid);
	/*MYSQL句柄*/
	MYSQL sqlfd;
	/*日表名*/
	std::string dayTableName;
	/*客户端输入的json*/
	Json::Value root;
	/*客户端套接字*/
	int sock;
	/*查询语句缓存*/
	char sqlbuffer[2048];
};

class DevRZ:public RZ
{
public:
	/*查询并反馈日表*/	
	bool Select(MYSQL &sqlfd,std::string tablename,Json::Value &root,int sock);
private:
	/*获取时间结构体*/
	Json::Value selectTimeInfo(const std::string & ipaddr,const std::string &devid);
	/*MYSQL句柄*/
	MYSQL sqlfd;
	/*日表名*/
	std::string dayTableName;
	/*客户端输入的json*/
	Json::Value root;
	/*客户端套接字*/
	int sock;
	/*查询语句缓存*/
	char sqlbuffer[2048];
};

template<typename QueryType>
class RZFX{
public:
	QueryType queryObj;
	bool Select(MYSQL &sqlfd,std::string tablename,Json::Value &root,int sock){
		 queryObj.Select(sqlfd,tablename,root,sock);
	}
};
const int breakTimeValue = 10*60;//飞机断开时间，大于600s视为断开
/*
class A{
public:
	void fun(void); 
private:
	int value;
};

class B{
private:
	std::vector<int> vectorInt;
	A a;
};*/

#endif
