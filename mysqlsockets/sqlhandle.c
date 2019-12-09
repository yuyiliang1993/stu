#include "sqlhandle.h"
#include "decodeData.h"

#include "decoderHead.h" //in codex
#include "PackBase.h"//in codex
#include "Prot_TH.h" //in codex

#include "lshf.h"
#include "myzlog.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <string>
#include "client.h"
using namespace std;
const char *table_name_user = "userInfo"; 
const char *table_name_plane = "planeInfo";
const char *table_name_company = "companyInfo";
const char *table_name_pilot = "pilotInfo";
const char *table_name_waypoint = "waypointInfo";
const char *table_name_airline = "airlineInfo";
const char *table_name_airspace = "airspaceInfo";
const char *table_name_dev = "devInfo";
const char *table_name_task = "taskInfo";
const char *table_name_datasrc = "datasrcInfo";
const char *table_name_devtype = "devtypeInfo";
extern int connSocket1;
extern std::string tServerIdnums;
MYSQL_RES *query_res(MYSQL &sqlfd,const char *sqlbuf,int sqllen);
extern int sendJson(int fd,Json::Value root);
extern bool string_to_json(const char*str,Json::Value &val);
void showHex(const char *buffer,int len)
{
		printf("hex:");
		for(int i =0;i<len;++i){
			printf("%02x ",(unsigned char )buffer[i]);
		}
		printf("\n");
}

static char * HEX(const char *buf,int len)
{
	static int maxlen = 2048;
	static char *buffer = new char[maxlen];
	int newlen = 2*len;
	if(newlen > maxlen){
		delete[]buffer;
		buffer = new char[newlen];
		maxlen = newlen;
	}
	int nlen = 0;
	for(int i = 0;i<len;++i){
		nlen += snprintf(buffer+nlen,maxlen-nlen,"%02x",(unsigned char)buf[i]);
	}
	return buffer;
}

SqlNet::SqlNet()
{
	if(::mysql_init(&sqlfd) == NULL){
		std::cerr<<"mysql_init failed!"<<std::endl;
		exit(1);
	}
}

SqlNet::~SqlNet()
{
	::mysql_close(&sqlfd);
}

bool SqlNet::Connect(const char * ip, const char * user, const char * passwd, const char * dbname)
{
	if(mysql_real_connect(&sqlfd,ip,user,passwd,dbname,0,NULL,0) == NULL){
		return false;
	}
	return true;
}

bool SqlNet::SetOptReconnect(void)
{
	int value =1;
	if(mysql_options(&sqlfd,MYSQL_OPT_RECONNECT,&value)){
		return false;
	}
	return true;
}

void SqlNet::close(){
	mysql_close(&sqlfd);
}


MYSQL &SqlNet::GetFd(void)
{
	return sqlfd;
}

bool SqlNet::IsConnectionBreak(void)
{
	return mysql_ping(&sqlfd)?true:false;
}

bool SqlNet::query(const char *sqlbuf,int sqllen)
{
	if(mysql_real_query(&sqlfd,sqlbuf,sqllen)){
		LOG(ERROR,"query err:%s,%s",sqlbuf,mysql_error(&sqlfd));
		return false;
	}
	return true;
}

int SqlNet::queryRows(const char * sqlbuf, int sqllen)
{
	if(!query(sqlbuf,sqllen)){
		return 0;
	}

	MYSQL_RES *res = mysql_store_result(&sqlfd);
	if(res == NULL){
		LOG(DEBUG,"mysql_result error:%s,%s",sqlbuf,mysql_error(&sqlfd));
		return 0;
	}
	
	long nums = mysql_num_rows(res);
	mysql_free_result(res);
	return nums;	
}

MYSQL_RES *SqlNet::queryRes(const char *sqlbuf,int sqllen)
{
	if(!query(sqlbuf,sqllen)){
		LOG(ERROR,"query err:%s,%s",sqlbuf,mysql_error(&sqlfd));
		return NULL;
	}

	MYSQL_RES *res = mysql_store_result(&sqlfd);
	if(res == NULL){
		LOG(ERROR,"mysql_result error:%s,%s",sqlbuf,mysql_error(&sqlfd));
		return NULL;
	}
	return res;	
}

MYSQL_ROW SqlNet::getRow(MYSQL_RES *res)
{
	return mysql_fetch_row(res);
}

int SqlNet::getFields(MYSQL_RES *res)
{
	return mysql_num_fields(res);	
}

int SqlNet::getRowNums(MYSQL_RES *res){
	return mysql_num_rows(res);
}


void SqlNet::result_free(MYSQL_RES *res)
{
	if(res){
		mysql_free_result(res);
		res = NULL;
	}
}

SqlWork::SqlWork(int sql_buf_len)
{
	max_sql_buffer_len = sql_buf_len;
	sqlbuffer = new char[sql_buf_len];
	mclient = NULL;
}

SqlWork::~SqlWork(){
	if(sqlbuffer){
		delete[]sqlbuffer;
		sqlbuffer = 0;
	}
}

bool SqlWork::create_all_table(void)
{
	//用户
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	int sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(userid varchar(256) not null,\
		companyid varchar(256) not null,password varchar(256),permission int,planeids MEDIUMTEXT,\
		info TEXT,primary key(userid,companyid))ENGINE = InnoDB CHARSET=\"utf8\"",table_name_user);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(companyid varchar(256),\
		signcompanyids TEXT,info TEXT,primary key(companyid))ENGINE = InnoDB CHARSET=\"utf8\"",table_name_company);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	
	//航点
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(companyid varchar(256) \
		not null,idnum int not null,waypoint text,primary key(companyid,idnum))ENGINE = InnoDB CHARSET=\"utf8\"",table_name_waypoint);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	//航线
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(companyid varchar(256) \
		not null,idnum int not null,airline text,primary key(companyid,idnum))ENGINE = InnoDB CHARSET=\"utf8\"",table_name_airline);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	//空域
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(companyid varchar(256) \
		not null,idnum int not null,airspace text,primary key(companyid,idnum))ENGINE = InnoDB CHARSET=\"utf8\"",table_name_airspace);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}

	//飞机
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(planeid int \
		not null,companyid varchar(256),info TEXT,primary key(planeid))ENGINE = InnoDB CHARSET=\"utf8\"",table_name_plane);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	//飞行员
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(pilotid int not null,companyid varchar(256),\
	info TEXT,primary key(pilotid))ENGINE=InnoDB CHARSET=\"utf8\"",table_name_pilot);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	
	//设备
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(devid varchar(256),devtypeid int,companyid varchar(256),planeid int,\
		pilotid int,info TEXT,primary key(devid,devtypeid))ENGINE=InnoDB CHARSET=\"utf8\"",table_name_dev);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	//任务
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(taskid int not null,companyid varchar(256),\
		planeids text,info text,primary key(taskid,companyid))ENGINE=InnoDB CHARSET=\"utf8\"",table_name_task);
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}

	//数据源
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(ipaddr varchar(512) not null,\
		devtypeid int,info varchar(1024),primary key(ipaddr))ENGINE=InnoDB CHARSET=\"utf8\"",table_name_datasrc);	
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	
	//设备类型
	memset(sqlbuffer,0,SQL_BUFFER_LEN);
	sqllen = snprintf(sqlbuffer,SQL_BUFFER_LEN,"create table if not exists %s(devtypeid int not null,\
	info TEXT,primary key(devtypeid))ENGINE=InnoDB CHARSET=\"utf8\"",table_name_devtype);	
	if(!query(sqlbuffer,sqllen)){
		std::cout<<"create table faield:"<<mysql_error(&sqlfd)<<":"<<sqlbuffer<<std::endl;
		return false;
	}
	return true;
}



bool SqlWork ::initMySql(void)
{
//#ifdef DB_NAME_WEBDATAS
//	if(!Connect("127.0.0.1","root","shunying","ShunYingData")){
	if(!Connect("127.0.0.1","root","shunying","webdatas")){
		LOG(ERROR,"connect mysql failed:%s",mysql_error(&sqlfd));
		return false;
		exit(0);
	}
//#endif

	LOG(DEBUG, "connect mysql success!");

	if(!SetOptReconnect()){
		LOG(ERROR,"setreconnect faield:%s",mysql_error(&sqlfd));
		return false;
		exit(0);
	}
	LOG(DEBUG, "set reconnect mysql success!");

	if(!create_all_table()){
		LOG(ERROR, "create table failed!");
		return false;
		exit(0);
	}
	LOG(DEBUG,"create all tables success!");
	return true;
}

bool SqlWork ::checkjson(Json :: Value & root)
{
#ifdef __DEBUG__
	Json::FastWriter writer;
	std::string data = writer.write(root);
	if(data[data.size()-1] == 0x0a)
		data[data.size()-1] = 0;
	LOG(DEBUG,"recv:%s",data.c_str());
#endif
	if(!root.isMember("type") || !root["type"].isString()){
		return false;
	}
	if(!root.isMember("content") || !root["content"].isObject()){
		return false;
	}
	return true;
}

bool SqlWork ::query_analysis(Json::Value &val,int sock,const ManageClients &manageclient)
{
	mclient = &manageclient;
	if(IsConnectionBreak()){//检查连接是否断开
		if(!initMySql())
			return false;
		LOG(WARN,"myql connection go away");
	}
	else if(!checkjson(val))
		return false;
	query_handle(val,sock);
	return true;
}

bool SqlWork ::	query_handle(Json::Value &val,int sock)
{
	std::string type = val["type"].asString();
	
	Json::Value &content = val["content"];

	if(type == "SSFX"){
		static class SSFX*ssfx = SSFX::getInstance();
		if(mclient)
			return ssfx->saveAndTransmit(sqlfd,content,*mclient);
	}
	LOG(DEBUG,"type:%s",type.c_str());

	if(type == "SX"){
		return SXSJ(content,sock);
	}
	
	if(type == "LSHF"){
		LSHF *lshf = LSHF::getInstance();
		return lshf->start(sqlfd,content,sock);
	}
	if(type == "TZHF"){
		LSHF *lshf = LSHF::getInstance();
		return lshf->stop(content);
	}
	
	if(type == "RZFX"){
		return RZFX(content,sock);
	}
	else if(type == "YHLB"){
		return YHLB(content,sock);
	}
	else if(type == "GSLB"){
		return GSLB(content,sock);
	}
	
	else if(type == "SBLB"){
		return SBLB(content,sock);
	}
	else if(type == "SBLXLB"){
		return SBLXLB(content,sock);
	}
	else if(type == "SJYLB"){
		return SJYLB(content,sock);
	}
	else if(type == "FXYLB"){
		return FXYLB(content,sock);
	}
	else if(type == "FJLB"){
		return FJLB(content,sock);
	}
/*用户*/
	else if(type == "YHZJ"){
		return YHZJ(content, sock);
	}
	else if(type == "YHSC"){
		return YHSC(content, sock);
	}
	else if(type == "YHXG"){
		return YHXG(content, sock);
	}
	else if(type == "YHCX"){
		return YHCX(content, sock);
	}
	
/*公司*/
	else if(type == "GSZJ"){
		return GSZJ(content, sock);
	}
	else if(type == "GSSC"){
		return GSSC(content, sock);
	}
	else if(type == "GSXG"){
		return GSXG(content, sock);
	}
	else if(type == "GSCX"){
		return GSCX(content, sock);
	}

/*航点*/
	else if(type == "HDZJ"){
		return HDZJ(content, sock);
	}
	else if(type == "HDSC"){
		return HDSC(content, sock);
	}
	else if(type == "HDXG"){
		return HDXG(content, sock);
	}
	else if(type == "HDCX"){
		return HDCX(content, sock);
	}
	
/*航线*/

	else if(type == "HXZJ"){
		return HXZJ(content, sock);
	}
	else if(type == "HXSC"){
		return HXSC(content, sock);
	}
	else if(type == "HXXG"){
		return HXXG(content, sock);
	}
	else if(type == "HXCX"){
		return HXCX(content, sock);
	}
	
/*空域*/

	else if(type == "KYZJ"){
		return KYZJ(content, sock);
	}
	else if(type == "KYSC"){
		return KYSC(content, sock);
	}
	else if(type == "KYXG"){
		return KYXG(content, sock);
	}
	else if(type == "KYCX"){
		return KYCX(content, sock);
	}

/*飞机*/
	
	else if(type == "FJZJ"){
		return FJZJ(content, sock);
	}
	else if(type == "FJSC"){
		return FJSC(content, sock);
	}
	else if(type == "FJXG"){
		return FJXG(content, sock);
	}
	else if(type == "FJCX"){
		return FJCX(content, sock);
	}

/*飞行员*/
	else if(type == "FXYZJ"){
		return FXYZJ(content, sock);
	}
	else if(type == "FXYSC"){
		return FXYSC(content, sock);
	}
	else if(type == "FXYXG"){
		return FXYXG(content, sock);
	}
	else if(type == "FXYCX"){
		return FXYCX(content, sock);
	}
	
/*设备*/
	else if(type == "SBZJ"){
		return SBZJ(content, sock);
	}
	else if(type == "SBSC"){
		return SBSC(content, sock);
	}
	else if(type == "SBXG"){
		return SBXG(content, sock);
	}
	else if(type == "SBCX"){
		return SBCX(content, sock);
	}
/*任务*/	
	else if(type == "RWZJ"){
		return RWZJ(content, sock);
	}
	else if(type == "RWSC"){
		return RWSC(content, sock);
	}
	else if(type == "RWXG"){
		return RWXG(content, sock);
	}
	else if(type == "RWCX"){
		return RWCX(content, sock);
	}

/*设备类型*/
	else if(type == "SBLXZJ"){
		return SBLXZJ(content, sock);
	}
	else if(type == "SBLXSC"){
		return SBLXSC(content, sock);
	}
	else if(type == "SBLXXG"){
		return SBLXXG(content, sock);
	}
	else if(type == "SBLXCX"){
		return SBLXCX(content, sock);
	}

/*数据源*/
	else if(type == "SJYZJ"){
		return SJYZJ(content, sock);
	}
	else if(type == "SJYSC"){
		return SJYSC(content, sock);
	}
	else if(type == "SJYXG"){
		return SJYXG(content, sock);
	}
	else if(type == "SJYCX"){
		return SJYCX(content, sock);
	}
	else if(type == "MMCX"){
		return MMCX(content, sock);
	}
	return true;	
}

bool SqlWork ::SXSJ(Json::Value&root,int sock)
{
	if(root["ipaddr"].isNull() || !root["ipaddr"].isString()){
		LOG(ERROR,"ipaddr err:%s",root.toStyledString().c_str());
		return false;
	}
	else if(root["devid"].isNull() || !root["devid"].isString()){
		LOG(ERROR,"devid err:%s",root.toStyledString().c_str());
		return false;
	}
	else if(root["protocol"].isNull() || !root["protocol"].isString()){
		LOG(ERROR,"protocol err:%s",root.toStyledString().c_str());
		return false;
	}
	else if(root["data"].isNull() || !root["data"].isArray()){
		LOG(ERROR,"data err:%s",root.toStyledString().c_str());
		return false;
	}
	std::string ipaddr =root["ipaddr"].asString();
	std::string devid = root["devid"].asString();
	std::string  protocol = root["protocol"].asString();
	Json::Value data = root["data"];
	PTR<CProt_TH> _Prot_TH( dynamic_cast<CProt_TH *>( CProt_TH::Prot().operator->()));
	PTR<CProtocolShell::unShelledPack> _sendData_TH = _Prot_TH->newEncoder();
	_sendData_TH->protocol = protocol;
	_sendData_TH->senderID = 0;
	_sendData_TH->receiverID = atoi(devid.c_str());
	for(int i=0;i<data.size();++i){
		if(data[i]["pack"].isNull()||!data[i]["pack"].isString()){
			LOG(ERROR,"data[i][pack] is error:%s",data.toStyledString().c_str());
			continue;
		}
		if(data[i]["type"].isNull()||!data[i]["type"].isString()){
			LOG(ERROR,"data[i][type] is error:%s",data.toStyledString().c_str());
			continue;
		}
		std::string type = data[i]["type"].asString();
		if(type == "ZYWD"){
			PTR<CProtocolShell::unShelledPack> _zywd = _Prot_TH->newEncoder();
			if(_Prot_TH->m_zywd->encodeByJson(_zywd,data[i]["pack"].asString())){
				LOG(DEBUG,"encode success!");
				_sendData_TH->Pack->appendData( *( _zywd->Pack ));
			}
		}
		if(type == "KYXX"){
			PTR<CProtocolShell::unShelledPack> _kyxx = _Prot_TH->newEncoder();
			if (_Prot_TH->m_kyxx->encodeByJson(_kyxx,data[i]["pack"].asString())){
				LOG(DEBUG,"encode success!");
				_sendData_TH->Pack->appendData( *( _kyxx->Pack ));
			}
		}
	}
	std::vector<BYTE> _sendBytes_TH = _Prot_TH->shell( *_sendData_TH );
	char *buff = new char[_sendBytes_TH.size()+1];
	char* p(buff);
	for(vector<BYTE>::iterator iter = _sendBytes_TH.begin();iter!= _sendBytes_TH.end();++iter){
		*p++ = *iter;
	}
	Json::Value val;
	val["head"] = "WS";
	val["type"] = "SS";
	val["content"]["ipaddr"] = ipaddr;
	val["content"]["data"]["SC"] = HEX(buff, _sendBytes_TH.size());
	LOG(DEBUG,"SXSJ:%s",val.toStyledString().c_str());
	if(mclient){
		mclient->sendToClient(tServerIdnums,val);
	}
	delete[]buff;
}


bool SqlWork::backfailure(int fd,Json::Value &root,const std::string &type,const std::string &reason)
{
	Json::Value reval;
	reval["head"]="SW";
	reval["type"]=type;
	root["result"]["value"]=false;
	root["result"]["reason"]=reason;
	reval["content"]=root;
	return (sendJson(fd,reval)>0? true:false);
}

bool SqlWork::backsuccess(int fd,Json::Value &root,const std::string &type,const std::string &reason)
{
	Json::Value reval;
	reval["head"]="SW";
	reval["type"]=type;
	root["result"]["value"]=true;
	root["result"]["reason"]=reason;
	reval["content"]=root;
	return (sendJson(fd,reval)>0? true:false);	
}

bool SqlWork::YHZJ(Json::Value&root,int sock)
{
	string userid,companyid,password,planeids,info;
	int permission = 0;
	Json::FastWriter writer;
	std::string type ="YHZJ";
	if(!root.isMember("userid")){
		LOG(DEBUG,"need userid");
		backfailure(sock,root,type,"need userid");
		return false;
	}
	if(!root.isMember("companyid")){
		LOG(DEBUG,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["userid"].isString()){
		LOG(DEBUG,"userid is not string");
		backfailure(sock,root,type,"userid is not string");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(DEBUG,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
		
	userid = root["userid"].asString();
	companyid = root["companyid"].asString();
	LOG(DEBUG,"userid:%s",userid.c_str());
	LOG(DEBUG,"companyid:%s",companyid.c_str());
		
//检查双主键长度元素
	if(!userid.size()){
		LOG(ERROR,"size of userid is null");
		backfailure(sock,root,type,"size of userid is null");
		return false;		
	}
	if(!companyid.size()){
		LOG(ERROR,"size of companyid is null");
		backfailure(sock,root,type,"size of companyid is null");
		return false;		
	}
	
//检查userid、companyid对应的记录是否已经存在
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\' and userid=\'%s\'",
		table_name_user,companyid.c_str(),userid.c_str());
	if(queryRows(sqlbuffer,sqllen) > 0){
		LOG(ERROR,"userid for the companyid alreay existed");
		backfailure(sock,root,type,"userid for the companyid alreay existed");
		return false;
	}

	//检查companyid是否存在公司表中
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer, sqllen) <= 0){//表示表中无companyid的记录
		LOG(ERROR,"companyid is invalid");
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}

//解析password	
	if(!root["password"].isNull()){
		if(!root["password"].isString()){
			LOG(DEBUG,"password is not string");
			backfailure(sock,root,type,"password is not string");
			return false;
		}
		password = root["password"].asString();
	}

//解析出权限	
	if(!root["permission"].isNull()){
		if(!root["permission"].isInt()){
			LOG(DEBUG,"permission is not int");
			backfailure(sock,root,type,"permission is not int");
			return false;
		}
		permission = root["permission"].asInt();
	}
	
//解析planeids并判断有效性	
	if(!root["planeids"].isNull()){
		if(!root["planeids"].isArray()){
			LOG(DEBUG,"planeids is not array");
			backfailure(sock,root,type,"planeids is not array");
			return false;		
		}
		for(int i = 0;i < root["planeids"].size();++i){
			if(!root["planeids"][i].isInt()){
				LOG(DEBUG,"planeids is not int");
				backfailure(sock,root,type,"planeids is not int");
				return false;
			}
			int planeid = root["planeids"][i].asInt();
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid = %d and companyid=\'%s\'",
				table_name_plane,planeid,companyid.c_str());
			if(queryRows(sqlbuffer, sqllen) <= 0){
				LOG(ERROR,"planeids is invalid:%d",planeid);
				backfailure(sock,root,type,"无效的飞机号或飞机的公司标识与该用户的公司标识不一致");
				return false;
			}
		}
		planeids = writer.write(root["planeids"]);
		if(planeids[planeids.size()-1]=='\n')
			planeids[planeids.size()-1] = 0;
		
	}
	
//解析info
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(DEBUG,"info is not array");
			backfailure(sock,root,type,"info is not array");
			return false;
		}
		info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n')
			info[info.size()-1] = 0;
	}

//执行增加操作	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s()values(\'%s\',\'%s\',\'%s\',\'%d\',\'%s\',\'%s\')",
		table_name_user,userid.c_str(),companyid.c_str(),password.c_str(),permission,planeids.c_str(),info.c_str());
	if(!query(sqlbuffer,sqllen)){
		LOG(ERROR,"query:%s,%s",mysql_error(&sqlfd),sqlbuffer);
		backfailure(sock,root,type,"insert failed");
		return false;
	}
	backsuccess(sock,root,type,"inserted");
	LOG(DEBUG,"query:%s",sqlbuffer);
	return true;	
}


bool SqlWork::YHSC(Json::Value&root,int sock)
{
	string userid,companyid;
	std::string type ="YHSC";
	if(!root.isMember("userid")){
		LOG(DEBUG,"need userid");
		backfailure(sock,root,type,"need userid");
		return false;
	}
	if(!root.isMember("companyid")){
		LOG(DEBUG,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	
	if(!root["userid"].isString()){
		LOG(DEBUG,"userid is not string");
		backfailure(sock,root,type,"userid is not string");
		return false;
	}
	
	if(!root["companyid"].isString()){
		LOG(DEBUG,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	userid = root["userid"].asString();
	companyid = root["companyid"].asString();
	LOG(DEBUG,"userid:%s",userid.c_str());
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	//检查是否存在该userid、compnayid对应的记录

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\' and userid=\'%s\'",
	table_name_user,companyid.c_str(),userid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0 ){
		LOG(ERROR,"not found:%s",sqlbuffer);
		backfailure(sock,root,type,"not found record of the userid and companyid");
		return false;
	}
	
	//执行删除操作
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\' and userid=\'%s\'",
		table_name_user,companyid.c_str(),userid.c_str());
	if(!query(sqlbuffer,sqllen)){
		LOG(ERROR,"delete failed:%s",sqlbuffer);
		backfailure(sock,root,type,"delete failed");
		return false;
	}
	LOG(ERROR,"delete:%s",sqlbuffer);
	backsuccess(sock,root,type,"delete success");
	return true;
}

bool SqlWork::YHXG(Json::Value&root,int sock)
{	
	string userid,companyid,password,planeids,info;
	int permission = 0;
	std::string type ="YHXG";
	if(!root.isMember("userid")){
		LOG(DEBUG,"need userid");
		backfailure(sock,root,type,"need userid");
		return false;
	}
	if(!root.isMember("companyid")){
		LOG(DEBUG,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	
	if(!root["userid"].isString()){
		LOG(DEBUG,"userid is not string");
		backfailure(sock,root,type,"userid is not string");
		return false;
	}
	
	if(!root["companyid"].isString()){
		LOG(DEBUG,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	userid = root["userid"].asString();
	companyid = root["companyid"].asString();
	
	//检查是否存在对应的记录
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where userid=\'%s\' and companyid=\'%s\'",
		table_name_user,userid.c_str(),companyid.c_str());
	if(queryRows(sqlbuffer, sqllen) <= 0){
		LOG(ERROR,"not found:%s",sqlbuffer);
		backfailure(sock,root,type,"not found the record of the userid for companyid");
		return false;
	}

	int updatenums = 0;
	if(root.isMember("planeids")){
		if(!root["planeids"].isArray()){
			LOG(ERROR,"planeids is not array");
			backfailure(sock,root,type,"planeids is not array");
			return false;
		}
	//检查planids是否存在plane表中
		for(int i = 0;i<root["planeids"].size();++i){	
			if(!root["planeids"][i].isInt()){
				LOG(ERROR,"planeid is not int");
				backfailure(sock,root,type,"planeids is not int array");
				return false;	
			}
			int planeid = root["planeids"][i].asInt();
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid = %d and companyid=\'%s\'",
				table_name_plane,planeid,companyid.c_str());
			if(queryRows(sqlbuffer, sqllen) <= 0){
				LOG(ERROR,"planeids is invalid");
				backfailure(sock,root,type,"planeids is invalid");
				return false;
			}
		}
		
		Json::FastWriter writer;
		planeids = writer.write(root["planeids"]);
		if(planeids[planeids.size()-1]=='\n')
			planeids[planeids.size()-1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set planeids=\'%s\' where userid=\'%s\' and companyid=\'%s\'",
		table_name_user,planeids.c_str(),userid.c_str(),companyid.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update planeids failed");
			return false;
		}
		++updatenums;
	}
	
	if(root.isMember("password")){
		if(!root["password"].isString()){
			LOG(ERROR,"password is not string");
			backfailure(sock,root,type,"password is not string");
			return false;	
		}
		password = root["password"].asString();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set password=\'%s\' where userid=\'%s\' and companyid=\'%s\'",
		table_name_user,password.c_str(),userid.c_str(),companyid.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update password failed");
			return false;
		}
		++updatenums;
	}

	if(root.isMember("permission")){
		if(!root["permission"].isInt()){
			LOG(ERROR,"permission is not int");
			backfailure(sock,root,type,"permission is not int");
			return false;
		}
		
		permission = root["permission"].asInt();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set permission=\'%d\' where userid=\'%s\' and companyid=\'%s\'",
		table_name_user,permission,userid.c_str(),companyid.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update permission failed");
			return false;
		}
		++updatenums;
	}
	if(root.isMember("info")){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		
		Json::FastWriter writer;
		info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n')
			info[info.size()-1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set info =\'%s\' where userid=\'%s\' and companyid=\'%s\'",
		table_name_user,info.c_str(),userid.c_str(),companyid.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update info failed");
			return false;
		}
		++updatenums;
	}
	
	if(updatenums == 0){
		LOG(ERROR,"update failed");
		backfailure(sock,root,type,"update failed");
		return false;
	}
	LOG(DEBUG,"YHXG:updated");
	backsuccess(sock,root,type,"YHXG:updated");
	return true;
}

bool SqlWork::YHCX(Json::Value&root,int sock)
{
	string userid,companyid;
	std::string type ="YHCX";
	root["password"]="";
	root["permission"]=0;
	root["planeids"].resize(0);
	root["info"].isObject();

	if(!root.isMember("userid")){
		LOG(DEBUG,"need userid");
		backfailure(sock,root,type,"need userid");
		return false;
	}
	if(!root.isMember("companyid")){
		LOG(DEBUG,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	
	if(!root["userid"].isString()){
		LOG(DEBUG,"userid is not string");
		backfailure(sock,root,type,"userid is not string");
		return false;
	}
	
	if(!root["companyid"].isString()){
		LOG(DEBUG,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	userid = root["userid"].asString();
	companyid = root["companyid"].asString();	
	LOG(DEBUG,"userid:%s",userid.c_str());
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	
	int sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where userid=\'%s\' and companyid=\'%s\'",
	table_name_user,userid.c_str(),companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found:%s",sqlbuffer);
		backfailure(sock,root,type,"No record of the userid was found");
		return false;
	}

	MYSQL_RES *res = queryRes(sqlbuffer, sqllen);
	MYSQL_ROW row;
	Json::Value planeidsval;
	Json::Value infoval;
	if(res && (getFields(res)>=6) && (row=getRow(res))){
		root["password"] = "";
		root["permission"] = 0;
		root["planeids"]= planeidsval;
		root["info"] = infoval;
		if(row[2])
			root["password"] = row[2];
		if(row[3])
			root["permission"] = atoi(row[3]);
		if(row[4] && string_to_json(row[4],planeidsval)){
			root["planeids"] = planeidsval;
		}
		if(row[5] && string_to_json(row[5],infoval)){
			root["info"] = infoval;
		}
		LOG(DEBUG, "select:%s",sqlbuffer);
		backsuccess(sock,root,type,"select success");
		result_free(res);
		return true;
	}
	else{
		result_free(res);
		LOG(ERROR, "select failed:%s",sqlbuffer);
		backfailure(sock,root,type,"select failed");
		return false;
	}
	return true;
}



bool SqlWork:: GSZJ(Json::Value&root,int sock)
{
	string companyid,publishtasks,signcompanyids,info;
	int sqllen = 0;
	Json::FastWriter writer;
	std::string type="GSZJ";
	if(!root.isMember("companyid")){
		backfailure(sock,root,type,"need companyid");
		LOG(ERROR, "need companyid");
		return false;
	}
/*	if(!root.isMember("publishtasks")){
		backForClient(sock,0,root,"GSZJ","need publishtaskids");
		LOG(ERROR, "need publishtaskids");
		return false;
	}*/
/*	if(!root.isMember("signcompanyids")){
		backfailure(sock,root,type,"need signcompanyids");
		LOG(ERROR, "need signcompanyids");
		return false;
	}*/

/*	if(!root.isMember("info")){
		backfailure(sock,root,type,"need info");
		LOG(ERROR, "need info");
		return false;
	}*/
	
	if(!root["companyid"].isString()){
		backfailure(sock,root,type,"companyid is not string");
		LOG(ERROR, "companyid is not string");
		return false;	
	}

	companyid =  root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	
//检查对应的记录是否存在
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer, sqllen) > 0){
		backfailure(sock,root,type,"record of the companyid already existed");
		LOG(ERROR, "record of the companyid already existed:%s",sqlbuffer);
		return false;
	}
	
//检查publishtaskids	
/*	if(root["publishtasks"].isArray()){
		if(root["publishtasks"].size() == 0){
			publishtasks.clear();
		}else{
			publishtasks = writer.write(root["publishtasks"]);
			if(publishtasks[publishtasks.size()-1] == '\n')
				publishtasks[publishtasks.size()-1] = 0;
			for(int i =0;i<root["publishtasks"].size();++i){
				if(root["publishtasks"][i].isInt()){
					int taskid =  root["publishtasks"][i].asInt();
					sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where taskid=\'%d\'",table_name_task,taskid);
					if(queryRows(sqlbuffer,sqllen) <= 0){
						backForClient(sock,0,root,"GSZJ","publishtaskids is invalid");
						LOG(ERROR,"publishtaskids is invalid:%s",sqlbuffer);
						return false;
					}
				}
				else{
					backForClient(sock,0,root,"GSZJ","publishtaskids is invalid");
					LOG(ERROR,"root[publishtasks][i] is not int");
					return false;
				}
			}
		}
	}
	else{
		backForClient(sock,0,root,"GSZJ","publishtaskids is not array");
		LOG(ERROR, "publishtaskids is not array");
		return false;
	}
*/

	if(!root["signcompanyids"].isNull()){
		if(!root["signcompanyids"].isArray()){
			backfailure(sock,root,type,"signcompanyids is not array");
			LOG(ERROR, "signcompanyids is not array");
			return false;
		}
		for(int i = 0;i<root["signcompanyids"].size();++i){
			if(root["signcompanyids"][i].isString()){
				string companyid = root["signcompanyids"][i].asString();
				sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
				table_name_company,companyid.c_str());
				if(queryRows(sqlbuffer, sqllen) <= 0){
					backfailure(sock,root,type,"signcompanyids is invalid");
					LOG(ERROR,"signcompanyids is invalid:%s",sqlbuffer);
					return false;
				}
			}else{
				backfailure(sock,root,type,"signcompanyids is invalid");
				LOG(ERROR,"companyid of signcompanyids is not string");
				return false;
			}
		}
		signcompanyids = writer.write(root["signcompanyids"]);
		if(signcompanyids[signcompanyids.size()-1] == '\n')
			signcompanyids[signcompanyids.size()-1] = 0; 
	}
	


	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR, "info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n')
			info[info.size()-1] = 0;
	}
//执行增加操作	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%s\',\'%s\',\'%s\')",
	table_name_company,companyid.c_str(),signcompanyids.c_str(),info.c_str());
	if(query(sqlbuffer,sqllen) == false){
		backfailure(sock,root,type,"insert company failed");
		LOG(ERROR,"insert companyid failed:%s",sqlbuffer);
		return false;
	}
	backsuccess(sock,root,type,"insert success");
	LOG(DEBUG,"insert success");
	return true;
}


bool delete_user_companyid(MYSQL sqlfd,int sock,std::string &companyid,SqlWork&sqlwork)
{
	char querybuffer[1024];
	int querylen = snprintf(querybuffer,sizeof(querybuffer),"select userid from %s where companyid=\'%s\'",table_name_user,companyid.c_str());
	if(querylen < 0 ){
		LOG(ERROR, "snprintf Err:%s",strerror(errno));
		return false;
	}
	if(mysql_real_query(&sqlfd,querybuffer,querylen)){
		LOG(ERROR, "query Err:%s,%s",querybuffer,mysql_error(&sqlfd));
		return false;
	}
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	if(res = mysql_store_result(&sqlfd)){
		while((row = mysql_fetch_row(res)) && row[0]){
			Json::Value content;
			content["userid"] = row[0];
			content["companyid"] = companyid;
			sqlwork.YHSC(content,sock);
		}
		mysql_free_result(res);
	}
	return true;
}

bool update_plane_companyid(MYSQL sqlfd,int sock,std::string &companyid,SqlWork&sqlwork)
{
	char querybuffer[1024];
	int querylen = snprintf(querybuffer,sizeof(querybuffer),"select planeid from %s where companyid=\'%s\'",table_name_plane,companyid.c_str());
	if(querylen < 0 ){
		LOG(ERROR, "snprintf:%s",strerror(errno));
		return false;
	}
	if(mysql_real_query(&sqlfd,querybuffer,querylen)){
		LOG(ERROR, "query err:%s,%s",querybuffer,mysql_error(&sqlfd));
		return false;
	}
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	if(res = mysql_store_result(&sqlfd)){
		while((row = mysql_fetch_row(res)) && row[0]){
			Json::Value content;
			content["planeid"] = atoi(row[0]);
			content["companyid"] = "";
			sqlwork.FJXG(content,sock);
		}
		mysql_free_result(res);
	}
	return true;
}


bool update_pilot_companyid(MYSQL sqlfd,int sock,std::string &companyid,SqlWork&sqlwork)
{
	char querybuffer[1024];
	int querylen = snprintf(querybuffer,sizeof(querybuffer),"select pilotid from %s where companyid=\'%s\'",table_name_pilot,companyid.c_str());
	if(querylen < 0 ){
		LOG(ERROR, "snprintf:%s",strerror(errno));
		return false;
	}
	if(mysql_real_query(&sqlfd,querybuffer,querylen)){
		LOG(ERROR, "query err:%s,%s",querybuffer,mysql_error(&sqlfd));
		return false;
	}
	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	if(res = mysql_store_result(&sqlfd)){
		while((row = mysql_fetch_row(res)) && row[0]){
			Json::Value content;
			content["pilotid"] = atoi(row[0]);
			content["companyid"] = "";
			sqlwork.FXYXG(content,sock);
		}
		mysql_free_result(res);
	}
	return true;
}

bool delete_task_companyid(MYSQL sqlfd,int sock,std::string &companyid,SqlWork&sqlwork)
{
	char querybuffer[1024];
	int querylen = snprintf(querybuffer,sizeof(querybuffer),"select from %s where companyid=\'%s\'",table_name_user,companyid.c_str());
	if(querylen < 0 ){
		LOG(ERROR, "snprintf:%s",strerror(errno));
		return false;
	}
	if(mysql_real_query(&sqlfd,querybuffer,querylen)){
		LOG(ERROR, "query err:%s,%s",querybuffer,mysql_error(&sqlfd));
		return false;
	}
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	if(res = mysql_store_result(&sqlfd)){
		while((row = mysql_fetch_row(res)) && row[0]){
			Json::Value content;
			content["userid"] = row[0];
			content["companyid"] = companyid;
			if(!sqlwork.YHSC(content,sock)){
				mysql_free_result(res);
				return false;
			}
		}
		mysql_free_result(res);
	}
	return true;
}

bool update_companyid_signcompanyids(MYSQL sqlfd, int sock, std :: string & companyid, SqlWork & sqlwork)
{
	char querybuffer[1024];
	//更新公司表中所有签约公司	
	int	querylen = snprintf(querybuffer,sizeof(querybuffer),"select companyid,signcompanyids from %s",table_name_company);
	if(querylen < 0 ){
		LOG(ERROR, "snprintf:%s",strerror(errno));
		return false;
	}
	if(mysql_real_query(&sqlfd,querybuffer,querylen)){
		LOG(ERROR, "query err:%s,%s",querybuffer,mysql_error(&sqlfd));
		return false;
	}
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	if(res = mysql_store_result(&sqlfd)){
		while((row=mysql_fetch_row(res)) && row[0] && row[1]){
			Json::Value signcompanyids;
			if(string_to_json(row[1],signcompanyids) && signcompanyids.isArray() && (signcompanyids.size()>0) ){
				int existValue = 0;
				int index = 0;
				Json::Value newSignCompanyids;
				newSignCompanyids.resize(0);
				for(int i = 0;i<signcompanyids.size() && signcompanyids[i].isString();++i){
					if(signcompanyids[i].asString() == companyid){
						++existValue;
						continue;
					}
					newSignCompanyids.resize(index+1);
					newSignCompanyids[index] = signcompanyids[i];
					++index;
				}

				if(existValue){
					Json::Value content;
					content["companyid"] = row[0];
					content["signcompanyids"]=newSignCompanyids;
					if(sqlwork.GSXG(content,sock) == false){
						mysql_free_result(res);
						return false;
					}
				}
			}
		}
		mysql_free_result(res);
	}
	return true;
}

bool SqlWork:: GSSC(Json::Value&root,int sock)
{
	string companyid;
	std::string type = "GSSC";
	if(!root.isMember("companyid")){
		LOG(DEBUG,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(DEBUG,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid =  root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	//检查是否存在compoanyid的记录
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
	if(!query(sqlbuffer,sqllen)){
		backfailure(sock,root,type,"delete failed");
		LOG(DEBUG,"delete failed:%s",sqlbuffer);
		return false;
	}

	LOG(DEBUG,"GSSC:query:%s",sqlbuffer);

//删除用户表中companyid的记录
	delete_user_companyid(sqlfd,sock,companyid,*this);

//更新飞机表
	update_plane_companyid(sqlfd,sock,companyid,*this);

//更新飞行员表
	update_pilot_companyid(sqlfd,sock, companyid,*this);

//更新签约公司列表
	update_companyid_signcompanyids(sqlfd, sock,companyid, *this);
	
//删除所有任务
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\'",table_name_task,companyid.c_str());
	if(!query(sqlbuffer, sqllen)){
		LOG(DEBUG,"query err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
	}
	
//删除所有航点
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\'",table_name_waypoint,companyid.c_str());
	if(!query(sqlbuffer, sqllen)){
		LOG(DEBUG,"query err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
	}

//删除所有航线

	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\'",table_name_airline,companyid.c_str());
	if(!query(sqlbuffer, sqllen)){
		LOG(DEBUG,"query err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
	}
	
//删除所有空域	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\'",table_name_airspace,companyid.c_str());
	if(!query(sqlbuffer, sqllen)){
		LOG(DEBUG,"query err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
	}
	backsuccess(sock,root,type,"deleted");
	LOG(DEBUG,"delete companyid:%s",companyid.c_str());
	return true;
}

bool SqlWork:: GSXG(Json::Value&root,int sock)
{
	string companyid,publishtasks,signcompanyids,info;
	std::string type = "GSXG";
	if(!root.isMember("companyid")){
		LOG(DEBUG,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	
	if(!root["companyid"].isString()){
		LOG(DEBUG,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid =  root["companyid"].asString();
	LOG(DEBUG, "companyid:%s",companyid.c_str());
	
	Json::Value reval;
	reval["head"]="SW";
	reval["type"]="GSXG";
	//检查是否存在compoanyid的记录
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer, sqllen) <= 0){
		LOG(ERROR,"not found record of the companyid");
		backfailure(sock,root,type,"not found record of the companyid");
		return false;
	}
	
	Json::FastWriter writer;
	int updatenums = 0;
	if(root.isMember("signcompanyids")){
		if(!root["signcompanyids"].isArray()){
			LOG(ERROR,"signcompanyids is not array");
			backfailure(sock,root,type,"signcompanyids is not array");
			return false;			
		}

		for(int i = 0;i<root["signcompanyids"].size();++i){
			if(root["signcompanyids"][i].isString()){
				string signcompanyid = root["signcompanyids"][i].asString();
				sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
					table_name_company,signcompanyid.c_str());
				if(queryRows(sqlbuffer, sqllen) <= 0){
					LOG(ERROR, "signcompanyids is invalid:%s",sqlbuffer);
					backfailure(sock,root,type,"signcompanyids is invalid");
					return false;
				}
			}else{
				LOG(ERROR,"companyid of signcompanyids is not string");
				backfailure(sock,root,type,"companyid of signcompanyids is not string");
				return false;
			}
		}
		
		signcompanyids = writer.write(root["signcompanyids"]);
		if(signcompanyids[signcompanyids.size()-1] == '\n')
			signcompanyids[signcompanyids.size()-1] =0; 
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set signcompanyids=\'%s\' where companyid = \'%s\'",
			table_name_company,signcompanyids.c_str(),companyid.c_str());
		if(!query(sqlbuffer,sqllen)){
			backfailure(sock,root,type,"update signcompanyids failed");
			return false;
		}
		++updatenums;
	}
	if(root.isMember("info")){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		info = writer.write(root["info"]);
		if(info[info.size()-1]=='\n')
			info[info.size()-1] =0; 
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set info=\'%s\' where companyid = \'%s\'",
			table_name_company,info.c_str(),companyid.c_str());
		if(!query(sqlbuffer,sqllen)){
			LOG(ERROR, "update info failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update info failed");
			return false;
		}
		++updatenums;
	}

	if(updatenums == 0){
		LOG(ERROR, "update failed!");
		backfailure(sock,root,type,"update failed");
		return false;
	}
	LOG(ERROR, "update success!");
	backsuccess(sock,root,type,"update success");
	return true;
}

bool SqlWork:: GSCX(Json::Value&root,int sock)
{	
	root["signcompanyids"].resize(0);
	root["info"].isObject();
	
	string companyid;
	std::string type = "GSCX";
	if(!root.isMember("companyid")){
		LOG(DEBUG,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	
	if(!root["companyid"].isString()){
		LOG(DEBUG,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid =  root["companyid"].asString();
	LOG(DEBUG, "companyid:%s",companyid.c_str());
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer, sqllen) <= 0){
		backfailure(sock,root,type,"No record of the companyid was found");
		LOG(ERROR,"No record of the companyid was found:%s",sqlbuffer);
		return false;
	}
	Json::Value signcompanyidval;
	Json::Value info;
	MYSQL_RES * res = queryRes(sqlbuffer,sqllen);
	MYSQL_ROW row ;
	if(res && (getFields(res)>=3) && (row=getRow(res))){
		signcompanyidval.resize(0);
		root["signcompanyids"] = signcompanyidval;
		root["info"] = info;
		if(row[1] && string_to_json(row[1],signcompanyidval))
			root["signcompanyids"] = signcompanyidval;
		if(row[2] && string_to_json(row[2],info))
			root["info"] = info;
		backsuccess(sock,root,type,"select success");
		LOG(DEBUG,"select success");
		result_free(res);
		return true;
	}
	backfailure(sock,root,type,"select error");
	LOG(DEBUG,"select error:%s",sqlbuffer);
	result_free(res);
	return false;
}

static int getMaxIdnumHD(MYSQL *sqlfd,std::string companyid)
{
	char buffer[1024];
	MYSQL_ROW row;
	int maxidnum=0;
	MYSQL_RES *res = NULL;
	int sqllen=snprintf(buffer,1024,"select max(idnum) from %s where companyid=\'%s\'",table_name_waypoint,companyid.c_str());
	if(mysql_real_query(sqlfd,buffer,sqllen)){
		LOG(ERROR,"query err:%s",buffer);
		return -1;
	}
	res = mysql_store_result(sqlfd);
	if(res  && (row=mysql_fetch_row(res)) && row[0]){
		maxidnum=atoi(row[0]);
	}
	if(res)
		mysql_free_result(res);
	return maxidnum;
}

static int getMaxIdnumHX(MYSQL *sqlfd,std::string companyid)
{
	char buffer[1024];
	MYSQL_ROW row;
	int maxidnum=0;
	MYSQL_RES *res = NULL;
	int sqllen=snprintf(buffer,1024,"select max(idnum) from %s where companyid=\'%s\'",table_name_airline,companyid.c_str());
	if(mysql_real_query(sqlfd,buffer,sqllen)){
		LOG(ERROR,"query:%s",buffer);
		return -1;
	}
	res = mysql_store_result(sqlfd);
	if(res  && (row=mysql_fetch_row(res)) && row[0]){
		maxidnum=atoi(row[0]);
		cout<<"row[0]:"<<row[0]<<endl;
	}
	if(res)
		mysql_free_result(res);
	return maxidnum;
}

static int getMaxIdnumKY(MYSQL *sqlfd,std::string companyid)
{
	char buffer[1024];
	MYSQL_ROW row;
	int maxidnum=0;
	MYSQL_RES *res = NULL;
	int sqllen=snprintf(buffer,1024,"select max(idnum) from %s where companyid=\'%s\'",table_name_airspace,companyid.c_str());
	mysql_real_query(sqlfd,buffer,sqllen);
	res = mysql_store_result(sqlfd);
	if(res  && (row=mysql_fetch_row(res)) && row[0]){
		maxidnum=atoi(row[0]);
	}
	if(res)
		mysql_free_result(res);
	return maxidnum;
}


bool SqlWork::HDZJ(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "HDZJ";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
		
	companyid = root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"companyid is invalid");
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}

	if(!root["waypoints"].isNull()){
		if(!root["waypoints"].isArray()){
			LOG(ERROR,"waypoints is not array");
			backfailure(sock,root,type,"waypoints is not array");
			return false;
		}		
	}
	
	Json::Value waypoints = root["waypoints"];
	Json::FastWriter writer;
	int sum_insert = 0;
	int idnum = 0;
	root["idnums"].resize(waypoints.size());
	for(int i=0;i<waypoints.size();++i){
		std::string data = writer.write(root["waypoints"][i]);
		if(data[data.size()-1] == '\n'){
			data[data.size()-1] = 0;
		}
		idnum=getMaxIdnumHD(&sqlfd,companyid);
		if(idnum < 0){
			LOG(ERROR,"idnums error:%d",idnum);
			return -1;
		}
		
		++idnum;
		LOG(DEBUG,"idnums:%d",idnum);
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%s\',\'%d\',\'%s\')",
			table_name_waypoint,companyid.c_str(),idnum,data.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR, "insert failed:%s",sqlbuffer);
			goto INSERT_ERR;
		}
		root["idnums"][i] = idnum;
		++sum_insert;
	}
	LOG(DEBUG,"insert waypoints succcess");
	backsuccess(sock,root,type,"insert waypoints succcess");
	return true;
INSERT_ERR:
	for(int i = 0;i<sum_insert;++i){
		sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\' and idnum=\'%d\'",
			table_name_waypoint,companyid.c_str(),--idnum);
		query(sqlbuffer, sqllen);
	}
	LOG(ERROR,"insert waypoints failed");
	backfailure(sock,root,type,"insert waypoints failed");
	return false;
}

bool SqlWork::HDSC(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "HDSC";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_waypoint,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the companyid");
		backfailure(sock,root,type,"not found record of the companyid");
		return false;
	}
	
	if(!root["idnums"].isNull()){
		if(!root["idnums"].isArray()){
			LOG(ERROR,"idnums is not array");
			backfailure(sock,root,type,"idnums is not array");
			return false;
		}
	}

	if(root["idnums"].size() == 0){
		LOG(ERROR,"size of idnums is zero");
		backfailure(sock,root,type,"size of idnums is zero");
		return false;
	}
	
	for(int i=0;i<root["idnums"].size();++i){
		if(!root["idnums"][i].isInt()){
			LOG(ERROR,"idnum of idnums is not int");
			backfailure(sock,root,type,"idnum of idnums is not int");
			return false;
		}
		int idnum = root["idnums"][i].asInt();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_waypoint,companyid.c_str(),idnum);
		if(queryRows(sqlbuffer,sqllen) <=0){
			LOG(ERROR,"no select the idnums for the companyid:%s",sqlbuffer);
			backfailure(sock,root,type,"No record of idnum for the companyid was found");
			return false;
		}
		sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_waypoint,companyid.c_str(),idnum);
		if(query(sqlbuffer, sqllen) == false){
			LOG(ERROR,"delete failed:%s",sqlbuffer);
			backfailure(sock,root,type,"delete failed");
			return false;
		}
	}
	LOG(ERROR,"delete success");
	backsuccess(sock,root,type,"delete success");
	return true;
}
bool SqlWork::HDXG(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "HDXG";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_waypoint,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the companyid");
		backfailure(sock,root,type,"not found record of the companyid");
		return false;
	}

	if(!root.isMember("idnums") || !root["idnums"].isArray()){
		LOG(ERROR,"idnums is invalid");
		backfailure(sock,root,type,"idnums is invalid");
		return false;
	}

	if(!root.isMember("waypoints") || !root["waypoints"].isArray()){
		LOG(ERROR,"waypoints is invalid");
		backfailure(sock,root,type,"waypoints is invalid");
		return false;
	}
	Json::Value idnums = root["idnums"];
	Json::Value waypoints=root["waypoints"];
	if(waypoints.size() != idnums.size()){
		LOG(ERROR,"size of waypoints is not equal to size of idnums");
		backfailure(sock,root,type,"size of waypoints is not equal to size of idnums");
		return false;
	}
	Json::FastWriter writer;
	for(int i = 0;i<idnums.size();++i){
		if(!root["idnums"][i].isInt()){
			LOG(ERROR,"idnums is not int");
			backfailure(sock,root,type,"idnums is not int");
			return false;
		}
		int idnum = idnums[i].asInt();
		string waypoint = writer.write(waypoints[i]);
		if(waypoint[waypoint.size()-1]=='\n')
			waypoint[waypoint.size()-1] = 0;

		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_waypoint,companyid.c_str(),idnum);
		if(queryRows(sqlbuffer,sqllen) <=0){
			LOG(ERROR,"no select the idnums for the companyid:%s",sqlbuffer);
			backfailure(sock,root,type,"No record of idnum for the companyid was found");
			return false;
		}
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set waypoint=\'%s\' where companyid=\'%s\' and idnum=\'%d\'",
			table_name_waypoint,waypoint.c_str(),companyid.c_str(),idnum);
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update failed");
			return false;
		}
	}
	LOG(DEBUG, "update success");
	backsuccess(sock,root,type,"update success");
	return true;
}
bool SqlWork::HDCX(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "HDCX";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"companyid is invalid");
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
	table_name_waypoint,companyid.c_str());
	MYSQL_RES *res= queryRes(sqlbuffer, sqllen);
	MYSQL_ROW row;
	if(res){
		int rownums = getRowNums(res);
		LOG(DEBUG,"select %d rows",getRowNums(res));
		root["idnums"].resize(rownums);
		root["waypoints"].resize(rownums);
		int index = 0;
		Json::Value val;
		while(row=getRow(res)){
			if(row[1] && row[2] && string_to_json(row[2],val)){
				root["idnums"][index] = atoi(row[1]);
				root["waypoints"][index] = val;
			}
			++index;
		}
		result_free(res);
		LOG(DEBUG,"select waypoints success");
		backsuccess(sock,root, type,"select waypoints success");
		return true;
	}
	LOG(ERROR,"select waypoints failed");
	backfailure(sock,root,type,"select waypoints failed");
	return false;
}


bool SqlWork::HXZJ(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "HXZJ";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid = root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"companyid is invalid");
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}

	if(!root["airlines"].isNull()){
		if(!root["airlines"].isArray()){
			LOG(ERROR,"airlines is not array");
			backfailure(sock,root,type,"airlines is not array");
			return false;
		}		
	}
	Json::FastWriter writer;
	int sum_insert = 0;
	int idnum = -1;
	root["idnums"].resize(root["airlines"].size());
	for(int i=0;i<root["airlines"].size();++i){
		std::string data = writer.write(root["airlines"][i]);
		if(data[data.size()-1] == '\n'){
			data[data.size()-1] = 0;
		}
		idnum =getMaxIdnumHX(&sqlfd,companyid);
		if(idnum < 0){
			LOG(ERROR,"create idnums failed:%d",idnum);
			backfailure(sock,root,type,"create idnums failed");
			return false;
		}
		++idnum;
		LOG(DEBUG,"idnums:%d",idnum);
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%s\',\'%d\',\'%s\')",
			table_name_airline,companyid.c_str(),idnum,data.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR, "insert failed:%s",sqlbuffer);
			backfailure(sock,root,type,"insert failed");
			return false;
		}
		root["idnums"][i] = idnum;
		++sum_insert;
	}
	LOG(DEBUG,"insert airlines succcess");
	backsuccess(sock,root,type,"insert airlines succcess");
	return true;
INSERT_ERR:
	for(int i = 0;i<sum_insert;++i){
		sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\' and idnum=\'%d\'",
			table_name_airline,companyid.c_str(),--idnum);
		query(sqlbuffer, sqllen);
	}
	LOG(DEBUG,"insert airlines failed");
	backfailure(sock,root,type,"insert airlines failed");
	return false;
}

bool SqlWork::HXSC(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "HXSC";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_airline,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the companyid");
		backfailure(sock,root,type,"not found record of the companyid");
		return false;
	}
	
	if(!root["idnums"].isNull()){
		if(!root["idnums"].isArray()){
			LOG(ERROR,"idnums is not array");
			backfailure(sock,root,type,"idnums is not array");
			return false;
		}
	}

	if(root["idnums"].size() == 0){
		LOG(ERROR,"size of idnums is zero");
		backfailure(sock,root,type,"size of idnums is zero");
		return false;
	}
	
	for(int i=0;i<root["idnums"].size();++i){
		if(!root["idnums"][i].isInt()){
			LOG(ERROR,"idnum of idnums is not int");
			backfailure(sock,root,type,"idnum of idnums is not int");
			return false;
		}
		int idnum = root["idnums"][i].asInt();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_airline,companyid.c_str(),idnum);
		if(queryRows(sqlbuffer,sqllen) <=0){
			LOG(ERROR,"no select the idnums for the companyid:%s",sqlbuffer);
			backfailure(sock,root,type,"not found record of idnum for the companyid");
			return false;
		}
		sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_airline,companyid.c_str(),idnum);
		if(query(sqlbuffer, sqllen) == false){
			LOG(ERROR,"delete failed:%s",sqlbuffer);
			backfailure(sock,root,type,"delete failed");
			return false;
		}
	}
	LOG(ERROR,"delete success");
	backsuccess(sock,root,type,"delete success");
	return true;
}

bool SqlWork::HXXG(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "HXXG";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_airline,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the companyid");
		backfailure(sock,root,type,"not found record of the companyid");
		return false;
	}

	if(!root.isMember("idnums") || !root["idnums"].isArray()){
		LOG(ERROR,"idnums is invalid");
		backfailure(sock,root,type,"idnums is invalid");
		return false;
	}

	if(!root.isMember("airlines") || !root["airlines"].isArray()){
		LOG(ERROR,"airlines is invalid");
		backfailure(sock,root,type,"airlines is invalid");
		return false;
	}
	
	Json::Value idnums = root["idnums"];
	Json::Value airlines =root["airlines"];
	if(airlines.size() != idnums.size()){
		LOG(ERROR,"size of arlines is not equal to size of idnums");
		backfailure(sock,root,type,"size of airlines is not equal to size of idnums");
		return false;
	}
	
	Json::FastWriter writer;
	for(int i = 0;i<idnums.size();++i){
		if(!root["idnums"][i].isInt()){
			LOG(ERROR,"idnums is not int");
			backfailure(sock,root,type,"idnums is not int");
			return false;
		}
		int idnum = idnums[i].asInt();
		string airline = writer.write(airlines[i]);
		if(airlines[airlines.size()-1]=='\n')
			airlines[airlines.size()-1] = 0;

		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_airline,companyid.c_str(),idnum);
		if(queryRows(sqlbuffer,sqllen) <=0){
			LOG(ERROR,"not found the idnums for the companyid:%s",sqlbuffer);
			backfailure(sock,root,type,"not found the idnums for the companyid");
			return false;
		}
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set airline=\'%s\' where companyid=\'%s\' and idnum=\'%d\'",
			table_name_airline,airline.c_str(),companyid.c_str(),idnum);
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update failed");
			return false;
		}
	}
	LOG(DEBUG, "update success");
	backsuccess(sock,root,type,"update success");
	return true;

}

bool SqlWork::HXCX(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "HXCX";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"companyid is invalid");
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
	table_name_airline,companyid.c_str());
	MYSQL_RES *res= queryRes(sqlbuffer, sqllen);
	MYSQL_ROW row;
	if(res){
		int rownums = getRowNums(res);
		LOG(DEBUG,"select %d rows",getRowNums(res));
		root["idnums"].resize(rownums);
		root["airlines"].resize(rownums);
		int index = 0;
		Json::Value val;
		while(row=getRow(res)){
			if(row[1] && row[2] && string_to_json(row[2],val)){
				root["idnums"][index] = atoi(row[1]);
				root["airlines"][index] = val;
			}
			++index;
		}
		result_free(res);
		LOG(DEBUG,"select airlines success");
		backsuccess(sock,root, type,"select airlines success");
		return true;
	}
	LOG(ERROR,"select airlines failed");
	backfailure(sock,root,type,"select airlines failed");
	return false;
}


bool SqlWork::KYZJ(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "KYZJ";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid = root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"companyid is invalid");
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}
	
	if(!root["airspaces"].isNull()){
		if(!root["airspaces"].isArray()){
			LOG(ERROR,"airlines is not array");
			backfailure(sock,root,type,"airlines is not array");
			return false;
		}		
	}
		
	Json::FastWriter writer;
	int sum_insert = 0;
	int idnum = -1;
	root["idnums"].resize(root["airspaces"].size());
	for(int i=0;i<root["airspaces"].size();++i){
		std::string data = writer.write(root["airspaces"][i]);
		if(data[data.size()-1] == '\n'){
			data[data.size()-1] = 0;
		}
		idnum =getMaxIdnumKY(&sqlfd,companyid);
		if(idnum < 0){
			LOG(ERROR,"create idnums failed:%d",idnum);
			backfailure(sock,root,type,"create idnums failed");
			return false;
		}
		++idnum;
		LOG(DEBUG,"idnums:%d",idnum);
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%s\',\'%d\',\'%s\')",
			table_name_airspace,companyid.c_str(),idnum,data.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR, "insert failed:%s",sqlbuffer);
			backfailure(sock,root,type,"insert failed");
			return false;
		}
		root["idnums"][i] = idnum;
		++sum_insert;
	}
	LOG(DEBUG,"insert airspaces succcess");
	backsuccess(sock,root,type,"insert airspaces succcess");
	return true;
INSERT_ERR:
	for(int i = 0;i<sum_insert;++i){
		sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\' and idnum=\'%d\'",
			table_name_airspace,companyid.c_str(),--idnum);
		query(sqlbuffer, sqllen);
	}
	LOG(DEBUG,"insert airspaces failed");
	backfailure(sock,root,type,"insert airspaces failed");
	return false;
}

bool SqlWork::KYSC(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "KYSC";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_airspace,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the companyid");
		backfailure(sock,root,type,"not found record of the companyid");
		return false;
	}
	
	if(!root["idnums"].isNull()){
		if(!root["idnums"].isArray()){
			LOG(ERROR,"idnums is not array");
			backfailure(sock,root,type,"idnums is not array");
			return false;
		}
	}

	if(root["idnums"].size() == 0){
		LOG(ERROR,"size of idnums is zero");
		backfailure(sock,root,type,"size of idnums is zero");
		return false;
	}
	
	for(int i=0;i<root["idnums"].size();++i){
		if(!root["idnums"][i].isInt()){
			LOG(ERROR,"idnum of idnums is not int");
			backfailure(sock,root,type,"idnum of idnums is not int");
			return false;
		}
		int idnum = root["idnums"][i].asInt();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_airspace,companyid.c_str(),idnum);
		if(queryRows(sqlbuffer,sqllen) <=0){
			LOG(ERROR,"not found record of idnum for the companyid:%s",sqlbuffer);
			backfailure(sock,root,type,"not found record of idnum for the companyid");
			return false;
		}
		sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_airspace,companyid.c_str(),idnum);
		if(query(sqlbuffer, sqllen) == false){
			LOG(ERROR,"delete failed:%s",sqlbuffer);
			backfailure(sock,root,type,"delete failed");
			return false;
		}
	}
	LOG(ERROR,"delete success");
	backsuccess(sock,root,type,"delete success");
	return true;

}

bool SqlWork::KYXG(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "KYXG";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_airspace,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the companyid");
		backfailure(sock,root,type,"not found record of the companyid");
		return false;
	}

	if(!root.isMember("idnums") || !root["idnums"].isArray()){
		LOG(ERROR,"idnums is invalid");
		backfailure(sock,root,type,"idnums is invalid");
		return false;
	}

	if(!root.isMember("airspaces") || !root["airspaces"].isArray()){
		LOG(ERROR,"airspaces is invalid");
		backfailure(sock,root,type,"airspaces is invalid");
		return false;
	}
	
	Json::Value idnums = root["idnums"];
	Json::Value airspaces=root["airspaces"];
	if(airspaces.size() != idnums.size()){
		LOG(ERROR,"size of airspaces is not equal to size of idnums");
		backfailure(sock,root,type,"size of airspaces is not equal to size of idnums");
		return false;
	}
	
	Json::FastWriter writer;
	for(int i = 0;i<idnums.size();++i){
		if(!root["idnums"][i].isInt()){
			LOG(ERROR,"idnums is not int");
			backfailure(sock,root,type,"idnums is not int");
			return false;
		}
		int idnum = idnums[i].asInt();
		string airspace = writer.write(airspaces[i]);
		if(airspaces[airspaces.size()-1]=='\n')
			airspaces[airspaces.size()-1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\' and idnum=\'%d\'",
		table_name_airspace,companyid.c_str(),idnum);
		if(queryRows(sqlbuffer,sqllen) <=0){
			LOG(ERROR,"not found the idnums for the companyid:%s",sqlbuffer);
			backfailure(sock,root,type,"not found the idnums for the companyid");
			return false;
		}
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set airspace=\'%s\' where companyid=\'%s\' and idnum=\'%d\'",
			table_name_airspace,airspace.c_str(),companyid.c_str(),idnum);
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update failed");
			return false;
		}
	}
	LOG(DEBUG, "update success");
	backsuccess(sock,root,type,"update success");
	return true;

}

bool SqlWork::KYCX(Json::Value&root,int sock)
{
	std::string companyid;	
	std::string type = "KYCX";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	companyid=root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
		table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"companyid is invalid");
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
	table_name_airspace,companyid.c_str());
	MYSQL_RES *res= queryRes(sqlbuffer, sqllen);
	MYSQL_ROW row;
	if(res){
		int rownums = getRowNums(res);
		LOG(DEBUG,"select %d rows",getRowNums(res));
		root["idnums"].resize(rownums);
		root["airspaces"].resize(rownums);
		int index = 0;
		Json::Value val;
		while(row=getRow(res)){
			if(row[1] && row[2] && string_to_json(row[2],val)){
				root["idnums"][index] = atoi(row[1]);
				root["airspaces"][index] = val;
			}
			++index;
		}
		result_free(res);
		LOG(DEBUG,"select airspaces success");
		backsuccess(sock,root, type,"select airspaces success");
		return true;
	}
	LOG(ERROR,"select airspaces failed");
	backfailure(sock,root,type,"select airspaces failed");
	return false;
}


static int getMaxPlaneId(MYSQL *sqlfd)
{
	char buffer[1024];
	int maxidnum=0;
	MYSQL_RES *res = NULL;
	int sqllen=snprintf(buffer,1024,"select max(planeid) from %s",table_name_plane);
	if(mysql_real_query(sqlfd,buffer,sqllen)){
		LOG(ERROR,"query failed:%s,%s",buffer,mysql_error(sqlfd));
		return -1;
	}
	res = mysql_store_result(sqlfd);
	if(res == NULL){
		LOG(ERROR,"result failed:%s,%s",buffer,mysql_error(sqlfd));
		return -1;
	}
	MYSQL_ROW row;
	if((row=mysql_fetch_row(res)) && row[0]){
		maxidnum = atoi(row[0]);
	}
	if(res)
		mysql_free_result(res);
	return maxidnum;
}

bool SqlWork::FJZJ(Json::Value&root,int sock)
{
	std::string companyid,info;
	std::string type="FJZJ";
	int sqllen = 0;
//不为空时，检查有效性
	if(!root["companyid"].isNull()){
		if(!root["companyid"].isString()){
			LOG(ERROR,"companyid is not string");
			backfailure(sock,root,type,"companyid is not string");
			return false;	
		}
		companyid = root["companyid"].asString();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
			table_name_company,companyid.c_str());
		if(queryRows(sqlbuffer,sqllen) <= 0){
			LOG(ERROR, "not found the companyid:%s",sqlbuffer);
			backfailure(sock,root,type,"not found the companyid");
			return false;
		}
	}
	
//info不为空时，需检查有效性	
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		info = writer.write(root["info"]);
		if(info[info.size() - 1] == '\n')
			info[info.size() - 1] = 0;
	}
	
	int planeid = getMaxPlaneId(&sqlfd);
	if(planeid < 0){
		LOG(ERROR, "create planeid was failed");
		backfailure(sock,root,type,"create planeid was failed");
		return false;
	}
	++planeid;
	LOG(DEBUG,"fjzj:planeid=%d",planeid);
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%d\',\'%s\',\'%s\')",
		table_name_plane,planeid,companyid.c_str(),info.c_str());
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR,"insert failed:%s,%s",mysql_error(&sqlfd),sqlbuffer);
		backfailure(sock,root,type,"insert failed");
		return false;
	}
	root["planeid"] = planeid;
	LOG(DEBUG,"insert success:%s",sqlbuffer);
	backsuccess(sock,root,type,"insert success");
	return true;
}

bool update_task_table_planeids(MYSQL &sqlfd,int planeid,int sock,SqlWork &sqlwork)
{
	const int maxbufferlen = 2048;
	char *sqlbuffer = new char[maxbufferlen];
	if(!sqlbuffer){
		LOG(ERROR,"new sqlbuffer err");
		return false;
	}
	
	int sqllen = snprintf(sqlbuffer,maxbufferlen,"select taskid,companyid,planeids from %s",table_name_task);
	if(mysql_real_query(&sqlfd,sqlbuffer,sqllen)){
		LOG(ERROR,"query err:%s",sqlbuffer);
		delete[] sqlbuffer;
		return false;
	}
	MYSQL_RES *res = NULL;
	if(res = mysql_store_result(&sqlfd)){
		MYSQL_ROW row;
		while((row=mysql_fetch_row(res)) && row[0] && row[1] && row[2]){
			Json::Value planeids;
			if(string_to_json(row[2],planeids) && planeids.isArray() && (planeids.size() > 0)){
				Json::Value newPlaneids;
				int index = 0;
				int existValue=0;
				newPlaneids.resize(index);
				for(int i = 0;i<planeids.size() && planeids[i].isInt();++i){
					if(planeids[i].asInt() == planeid){
						++existValue;
						continue;
					}
					newPlaneids.resize(index+1);
					newPlaneids[index] = planeids[i];
					++index;
				}
				if(existValue){
					Json::Value content;
					content["taskid"] = atoi(row[0]);
					content["companyid"] = row[1];
					content["planeids"]=newPlaneids;
					if(!sqlwork.RWXG(content,sock)){
						mysql_free_result(res);
						delete[] sqlbuffer;
						return false;	
					}
				}
			}
		}		
		mysql_free_result(res);
	}
	delete[] sqlbuffer;
	return true;
}

bool update_user_table_planeids(MYSQL &sqlfd,int planeid,int sock,SqlWork& sqlwork)
{
	const int maxbufferlen = 2048;
	//(1)申请动态内存
	char *sqlbuffer = new char[maxbufferlen];
	if(!sqlbuffer){
		LOG(ERROR,"new err");
		return false;
	}

	//(2)查询所有飞机用户列表
	int sqllen = snprintf(sqlbuffer,maxbufferlen,"select userid,companyid,planeids from %s",table_name_user);
	if(sqllen < 0){
		LOG(ERROR,"snprintf:%s",strerror(errno));
		delete[] sqlbuffer;
		return false;
	}
	
	if(mysql_real_query(&sqlfd,sqlbuffer,sqllen)){
		LOG(ERROR,"query err:%s",sqlbuffer);
		delete[] sqlbuffer;
		return false;
	}
	
	//(3)获取所有用户列表结果集
	MYSQL_RES *res = mysql_store_result(&sqlfd);
	if(res){
		MYSQL_ROW row;
		Json::Value planeids;
		while((row=mysql_fetch_row(res)) && row[0] && row[1] && row[2]){
			if(string_to_json(row[2],planeids) && planeids.isArray() && (planeids.size() > 0)){
				Json::Value newPlaneids;
				int index = 0;
				int existValue=0;
				newPlaneids.resize(index);
				for(int i = 0;i<planeids.size() && planeids[i].isInt();++i){
					if(planeids[i].asInt() == planeid){
						++existValue;
						continue;
					}
					newPlaneids.resize(index+1);
					newPlaneids[index] = planeids[i];
					++index;
				}
				if(existValue){
					Json::Value content;
					content["userid"] = row[0];
					content["companyid"] = row[1];
					content["planeids"] = newPlaneids;
					sqlwork.YHXG(content,sock);
				}
			}
		}
		mysql_free_result(res);
	}
	delete[] sqlbuffer;
	return true;
}

bool update_dev_table_planeid(MYSQL &sqlfd,int planeid,int sock,SqlWork & sqlwork)
{
	const int maxbufferlen = 2048;
	char *sqlbuffer = new char[maxbufferlen];
	if(!sqlbuffer){
		LOG(ERROR,"new sqlbuffer err");
		return false;
	}
	int sqllen =snprintf(sqlbuffer,maxbufferlen,"select devid,devtypeid from %s where planeid=%d",table_name_dev,planeid);
	if(sqllen < 0){
		LOG(ERROR,"snprintf:%s",strerror(errno));
		delete[]sqlbuffer;
		return false;
	}
	if(mysql_real_query(&sqlfd,sqlbuffer,sqllen)){
		LOG(ERROR,"query err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		delete[]sqlbuffer;
		return false;
	}
	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	if(res = mysql_store_result(&sqlfd)){
		while((row = mysql_fetch_row(res)) && row[0] &&row[1]){
			Json::Value content;
			content["devid"] = row[0];
			content["devtypeid"] = atoi(row[1]);
			content["planeid"] = -1;
			if(!sqlwork.SBXG(content,sock)){
				mysql_free_result(res);
				delete[] sqlbuffer;
				return false;
			}
		}
		mysql_free_result(res);
	}
	delete[] sqlbuffer;
	return true;
}

bool SqlWork::FJSC(Json::Value&root,int sock)
{
	int planeid = -1;
	std::string type="FJSC";
	if(!root.isMember("planeid")){
		LOG(ERROR,"need planeid");
		backfailure(sock,root,type,"need planeid");
		return false;	
	}
	if(!root["planeid"].isInt()){
		LOG(ERROR,"planeid is not int");
		backfailure(sock,root,type,"planeid is not int");
		return false;
	}
	planeid = root["planeid"].asInt();
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid=\'%d\'",table_name_plane,planeid);
/*
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found the planeid");
		backfailure(sock,root,type,"not found the planeid");
		return false;
	}
*/	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where planeid=\'%d\'",table_name_plane,planeid);
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR,"delete failed:%s",sqlbuffer);
		backfailure(sock,root,type,"delete failed");
		return false;
	}
	LOG(DEBUG,"delete plane:%s",sqlbuffer);
	backsuccess(sock,root,type,"");
//更新飞行数据表的飞机号

//更新设备表中的飞机号
	if(!update_dev_table_planeid(sqlfd,planeid,sock,*this)){
		LOG(ERROR,"update planeids of dev failed");
		backfailure(sock,root,type,"update planeids of dev failed");
		return false;
	}
	
//更新用户的飞机列表
	if(!update_user_table_planeids(sqlfd,planeid,sock,*this)){
		LOG(ERROR,"update planeids of user failed");
		backfailure(sock,root,type,"update planeids of user failed");
		return false;
	}
	
//更新任务表中的飞机列表
	if(!update_task_table_planeids(sqlfd,planeid,sock,*this)){
		LOG(ERROR,"update planeids of task failed");
		backfailure(sock,root,type,"update planeids of task failed");
		return false;
	}
	return true;
}

bool SqlWork::FJXG(Json::Value&root,int sock)
{
	std::string companyid,info;
	int planeid = -1;
	std::string type="FJXG";
	if(!root.isMember("planeid")){
		LOG(ERROR,"need planeid");
		backfailure(sock,root,type,"need planeid");
		return false;	
	}
	if(!root["planeid"].isInt()){
		LOG(ERROR,"planeid is not int");
		backfailure(sock,root,type,"planeid is not int");
		return false;
	}	
	planeid = root["planeid"].asInt();
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid=\'%d\'",table_name_plane,planeid);
	if(sqllen < 0){
		LOG(ERROR,"snprintf:%s",strerror(errno));
		return false;
	}
	
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found the planeid");
		backfailure(sock,root,type,"not found the planeid");
		return false;
	}
	
	int sucessflag = 0;
	if(root.isMember("companyid")){
		if(!root["companyid"].isString()){
			LOG(ERROR,"companyid is not string");
			backfailure(sock,root,type,"companyid is not string");
			return false;	
		}
		companyid = root["companyid"].asString();
		if(companyid.size() > 0){
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
				table_name_company,companyid.c_str());
			if(queryRows(sqlbuffer,sqllen) <= 0){
				LOG(ERROR,"companyid is invalid");
				backfailure(sock,root,type,"companyid is invalid");
				return false;
			}
		}
		//将旧的companyid查出保存
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select companyid from %s where planeid=\'%d\'",table_name_plane,planeid);
		MYSQL_RES * res = queryRes(sqlbuffer,sqllen);
		std::string companyid_old;
		if(res != NULL){
			MYSQL_ROW row = getRow(res);
			if(row && row[0]){
				 companyid_old = row[0];
			}
			result_free(res);
		}else{
			LOG(ERROR, "queryRes:%s",sqlbuffer);
		}
	
		//更新companyid
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set companyid =\'%s\' where planeid=\'%d\'",
		table_name_plane,companyid.c_str(),planeid);
		if(query(sqlbuffer, sqllen) == false){
			LOG(ERROR, "update companyid failed");
			backfailure(sock,root,type,"update companyid failed");
			return false;
		}
		LOG(DEBUG, "update companyid:%s",sqlbuffer);
		//修改companyid成功之后，需要更新用户的飞机列表
		if(companyid_old.size() > 0 && companyid != companyid_old){
			//(1)查询用户所有旧companyid对应的记录
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select planeids,userid from %s where companyid =\'%s\'",
			table_name_user,companyid_old.c_str());
			res = queryRes(sqlbuffer,sqllen);
			if(res == NULL){
				LOG(ERROR,"queryRes:%s",sqlbuffer);
				return false;
			}
			
			MYSQL_ROW row;
			Json::Value oPlaneids;//old
			Json::Value nPlaneids;//new
			//(2)开始修改飞机列表
			while((row =getRow(res))&& row[0] && string_to_json(row[0],oPlaneids) && row[1]){
				LOG(DEBUG,"start loop update planeids of user");
				int index = 0;
				int existValue = 0;
				nPlaneids.resize(0);
				for(int i =0;i<oPlaneids.size() && oPlaneids[i].isInt();++i){
					if(oPlaneids[i].asInt() == planeid){
						++existValue;
						continue;
					}
					nPlaneids.resize(index+1);
					nPlaneids[index] = oPlaneids[i].asInt();
					LOG(DEBUG,"nPlaneid:%d",nPlaneids[index].asInt());
					++index;
				}
				
				if(existValue){
					Json::Value content;
					content["userid"]= row[1];
					content["companyid"]=companyid_old;
					content["planeids"]=nPlaneids;
					YHXG(content,sock);
				}
			}
			result_free(res);
		}

		++sucessflag;
	}

	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		info = writer.write(root["info"]);
		if(info[info.size() - 1] == '\n')
			info[info.size() - 1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set info =\'%s\' where planeid=\'%d\'",
		table_name_plane,info.c_str(),planeid);
		if(query(sqlbuffer, sqllen))
			++sucessflag;
	}
	if(sucessflag == 0){
		LOG(DEBUG,"update failed");
		backfailure(sock,root,type,"update failed");
		return false;
	}
	LOG(DEBUG,"update:%s",sqlbuffer);
	backsuccess(sock,root,type,"updated");
	return true;
}

bool SqlWork::FJCX(Json::Value&root,int sock)
{
	int planeid = -1;
	std::string type="FJCX";
	if(!root.isMember("planeid")){
		LOG(ERROR,"need planeid");
		backfailure(sock,root,type,"need planeid");
		return false;	
	}
	if(!root["planeid"].isInt()){
		LOG(ERROR,"planeid is not int");
		backfailure(sock,root,type,"planeid is not int");
		return false;
	}
	planeid = root["planeid"].asInt();
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid=\'%d\'",table_name_plane,planeid);
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found the planeid");
		backfailure(sock,root,type,"not found the planeid");
		return false;
	}
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid=\'%d\'",
		table_name_plane,planeid);
	MYSQL_RES *res = queryRes(sqlbuffer, sqllen);
	MYSQL_ROW row;
	if(res && (row=getRow(res))){
		Json::Value val;
		root["companyid"]="";
		root["info"] = val;
		if(row[1])
			root["companyid"] = row[1];
		if(row[2] && string_to_json(row[2],val))
			root["info"] = val;
		LOG(DEBUG,"select success:%s",sqlbuffer);
		backsuccess(sock,root,type,"select success");
		result_free(res);
		return true;
	}	
	result_free(res);
	LOG(ERROR,"select failed:%s",sqlbuffer);
	backfailure(sock,root,type,"select failed");
	return true;
}

/*飞行员*/
static int getMaxPilotId(MYSQL *sqlfd)
{
	char buffer[1024];
	int maxidnum=0;
	MYSQL_RES *res = NULL;
	int sqllen=snprintf(buffer,1024,"select max(pilotid) from %s",table_name_pilot);
	if(mysql_real_query(sqlfd,buffer,sqllen)){
		LOG(ERROR,"query failed:%s,%s",buffer,mysql_error(sqlfd));
		return -1;
	}
	res = mysql_store_result(sqlfd);
	if(res == NULL){
		LOG(ERROR,"result failed:%s,%s",buffer,mysql_error(sqlfd));
		return -1;
	}
	MYSQL_ROW row;
	if((row=mysql_fetch_row(res)) && row[0]){
		maxidnum = atoi(row[0]);
	}
	if(res)
		mysql_free_result(res);
	return maxidnum;
}


bool SqlWork::FXYZJ(Json::Value&root,int sock)
{
	std::string companyid,info;
	std::string type="FXYZJ";
	int sqllen = 0;
//不为空时，检查有效性
	if(!root["companyid"].isNull()){
		if(!root["companyid"].isString()){
			LOG(ERROR,"companyid is not string");
			backfailure(sock,root,type,"companyid is not string");
			return false;	
		}
		companyid = root["companyid"].asString();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",
			table_name_company,companyid.c_str());
		if(queryRows(sqlbuffer,sqllen) <= 0){
			LOG(ERROR, "not found the companyid:%s",sqlbuffer);
			backfailure(sock,root,type,"not found the companyid");
			return false;
		}
	}
		
//info不为空时，需检查有效性 
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		info = writer.write(root["info"]);
		if(info[info.size() - 1] == '\n')
			info[info.size() - 1] = 0;
	}

	int pilotid = getMaxPilotId(&sqlfd);
	if(pilotid < 0){
		LOG(ERROR, "create pilotid is failed");
		backfailure(sock,root,type,"create pilotid is failed");
		return false;	
	}
	++pilotid;
	LOG(DEBUG,"pilotid:%d",pilotid);
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%d\',\'%s\',\'%s\')",
		table_name_pilot,pilotid,companyid.c_str(),info.c_str());
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR, "insert failed:%s",sqlbuffer);
		backfailure(sock,root,type,"insert failed");
		return false;	
	}
	root["pilotid"]=pilotid;
	LOG(DEBUG,"insert success");
	backsuccess(sock,root,type,"insert success");
	return true;	
}

bool update_dev_table_pilotid(MYSQL &sqlfd,int pilotid)
{
	const int maxbufferlen = 2048;
	char *sqlbuffer = new char[maxbufferlen];
	if(!sqlbuffer){
		LOG(ERROR,"new sqlbuffer err");
		return false;
	}
	int sqllen = snprintf(sqlbuffer,maxbufferlen,"update %s set pilotid=null where pilotid=%d",table_name_dev,pilotid);
	if(mysql_real_query(&sqlfd,sqlbuffer,sqllen)){
		LOG(ERROR,"query err:%s",sqlbuffer);
		delete[] sqlbuffer;
		return false;
	}
	delete[] sqlbuffer;
	return true;
}

bool update_dev_table_pilotid(MYSQL sqlfd,int pilotid,int sock,SqlWork & sqlwork)
{
	const int maxbufferlen = 2048;
	char *sqlbuffer = new char[maxbufferlen];
	if(!sqlbuffer){
		LOG(ERROR,"new sqlbuffer err");
		return false;
	}
	int sqllen =snprintf(sqlbuffer,maxbufferlen,"select devid,devtypeid from %s where pilotid=%d",table_name_dev,pilotid);
	if(sqllen < 0){
		LOG(ERROR,"snprintf:%s",strerror(errno));
		delete[]sqlbuffer;
		return false;
	}
	if(mysql_real_query(&sqlfd,sqlbuffer,sqllen)){
		LOG(ERROR,"query err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		delete[]sqlbuffer;
		return false;
	}
	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	if(res = mysql_store_result(&sqlfd)){
		while((row = mysql_fetch_row(res)) && row[0] &&row[1]){
#if 0
			sqllen = snprintf(sqlbuffer,maxbufferlen,"update %s set pilotid=null where devid=\'%s\' and devtypeid=\'%s\'",
				table_name_dev,row[0],row[1]);
			if(mysql_real_query(&sqlfd,sqlbuffer,sqllen)){
				LOG(ERROR,"query err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
				delete[]sqlbuffer;
				mysql_free_result(res);
				return false;
			}
#endif
			Json::Value content;
			content["devid"] = row[0];
			content["devtypeid"] = atoi(row[1]);
			content["pilotid"] = -1;
			if(sqlwork.SBXG(content,sock) == false){
				delete[] sqlbuffer;
				mysql_free_result(res);
				return false;
			}
	//		sqlwork.backsuccess(sock,content,"SBXG","update planeid of dev");
		}
		mysql_free_result(res);
	}
	delete[] sqlbuffer;
	return true;
}

bool SqlWork::FXYSC(Json::Value&root,int sock)
{
	int pilotid = 0;
	std::string type = "FXYSC";
	if(!root.isMember("pilotid")){
		LOG(ERROR, "need pilotid");
		backfailure(sock,root,type,"insert failed");
		return false;	
	}
	
	if(!root["pilotid"].isInt()){
		LOG(ERROR, "pilotid is not int");
		backfailure(sock,root,type,"pilotid is not int");
		return false;	
	}
	pilotid = root["pilotid"].asInt();
	LOG(DEBUG,"pilotid:%d",pilotid);

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where pilotid=\'%d\'",table_name_pilot,pilotid);
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR, "not found record of the pilotid");
		backfailure(sock,root,type,"not found record of the pilotid");
		return false;
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where pilotid=\'%d\'",table_name_pilot,pilotid);
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR, "delete failed:%s",sqlbuffer);
		backfailure(sock,root,type,"delete failed");
		return false;
	}
	LOG(DEBUG,"delete succcess:%s",sqlbuffer);
	backsuccess(sock,root,type,"delete success");
	//更新飞行数据表中的pilotid

	//更新设备表中的pilotid
//	update_dev_table_pilotid(sqlfd,pilotid);

	update_dev_table_pilotid(sqlfd,pilotid,sock,*this);
	
	return true;
}

bool SqlWork::FXYXG(Json::Value&root,int sock)
{
	std::string companyid,info;
	int pilotid = 0;
	std::string type = "FXYXG";
	if(!root.isMember("pilotid")){
		LOG(ERROR, "need pilotid");
		backfailure(sock,root,type,"need pilotid");
		return false;	
	}
	
	if(!root["pilotid"].isInt()){
		LOG(ERROR, "pilotid is not int");
		backfailure(sock,root,type,"pilotid is not int");
		return false;	
	}
	pilotid = root["pilotid"].asInt();
	LOG(DEBUG,"pilotid:%d",pilotid);

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where pilotid=\'%d\'",table_name_pilot,pilotid);
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR, "not found record of the pilotid");
		backfailure(sock,root,type,"not found record of the pilotid");
		return false;
	}

	if(root.isMember("companyid")){
		if(!root["companyid"].isString()){
			LOG(ERROR, "companyid is not string");
			backfailure(sock,root,type, "companyid is not string");
			return false;	
		}
		
		companyid = root["companyid"].asString();
		if(companyid.size() > 0){
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
			if(queryRows(sqlbuffer,sqllen) <= 0){
				LOG(ERROR,"companyid is invalid");
				backfailure(sock,root,type,"companyid is invalid");
				return false;
			}
		}
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set companyid=\'%s\' where pilotid=\'%d\'",
			table_name_pilot,companyid.c_str(),pilotid);
		if(query(sqlbuffer, sqllen) == false){
			LOG(ERROR, "update companyid failed");
			backfailure(sock,root,type, "update companyid failed");
			return false;
		}
	}
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n'){
			info[info.size()-1] = 0;
		}
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set info=\'%s\' where pilotid=\'%d\'",
			table_name_pilot,info.c_str(),pilotid);
		if(query(sqlbuffer, sqllen) == false){
			LOG(ERROR,"update info failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update info failed");
			return false;
		}
		
	}
	LOG(DEBUG,"update:%s",sqlbuffer);
	backsuccess(sock,root,type,"update success");
	return true;
}

bool SqlWork::FXYCX(Json::Value&root,int sock)
{
	int pilotid = 0;
	std::string type = "FXYCX";
	if(!root.isMember("pilotid")){
		LOG(ERROR, "need pilotid");
		backfailure(sock,root,type,"insert failed");
		return false;	
	}
	
	if(!root["pilotid"].isInt()){
		LOG(ERROR, "pilotid is not int");
		backfailure(sock,root,type,"pilotid is not int");
		return false;	
	}
	pilotid = root["pilotid"].asInt();
	LOG(DEBUG,"pilotid:%d",pilotid);

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where pilotid=\'%d\'",table_name_pilot,pilotid);
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR, "not found record of the pilotid");
		backfailure(sock,root,type,"not found record of the pilotid");
		return false;
	}

	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where pilotid=\'%d\'",table_name_pilot,pilotid);
 	MYSQL_RES *res = queryRes(sqlbuffer,sqllen);
	MYSQL_ROW row;
	if(res && (row = getRow(res))){
		Json::Value val;
		root["companyid"] = "";
		root["info"] = val;
		if(row[1])
			root["companyid"] = row[1];
		if(row[2] && string_to_json(row[2],val))
			root["info"] = val;
		LOG(DEBUG,"select success:%s",sqlbuffer);
		backsuccess(sock,root,type,"select success");
		result_free(res);
		return true;
	}
	
	LOG(ERROR, "select failed:%s",sqlbuffer);
	backfailure(sock,root,type,"select failed");
	result_free(res);
	return false;
}

static int getMaxTaskid(MYSQL *sqlfd,std::string companyid)
{
	char buffer[1024];
	int maxidnum=0;
	MYSQL_RES *res = NULL;
	int sqllen=snprintf(buffer,1024,"select max(taskid) from %s where companyid=\'%s\'",table_name_task,companyid.c_str());
	if(mysql_real_query(sqlfd,buffer,sqllen)){
		LOG(ERROR,"query failed:%s,%s",buffer,mysql_error(sqlfd));
		return -1;
	}
	res = mysql_store_result(sqlfd);
	if(res == NULL){
		LOG(ERROR,"result failed:%s,%s",buffer,mysql_error(sqlfd));
		return -1;
	}
	MYSQL_ROW row;
	if((row=mysql_fetch_row(res)) && row[0]){
		maxidnum = atoi(row[0]);
	}
	if(res)
		mysql_free_result(res);
	return maxidnum;
}

/*任务*/
bool SqlWork::RWZJ(Json::Value&root,int sock)
{
	string companyid,planeids,planetaskinfo,info;
	std::string type="RWZJ";
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}

//检查comapnyid有效性
	companyid = root["companyid"].asString();
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"companyid is invalid:%s",sqlbuffer);
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}
	
	Json::FastWriter writer;
//planeids 不为空时检查有效性
	if(!root["planeids"].isNull()){
		if(!root["planeids"].isArray()){
			LOG(ERROR,"planeids is not array");
			backfailure(sock,root,type,"planeids is not array");
			return false;
		}
		for(int i =0;i<root["planeids"].size();++i){
			if(!root["planeids"][i].isInt()){
				LOG(ERROR,"planeid of planeids is not int");
				backfailure(sock,root,type,"planeid of planeids is not int");
				return false;
			}
			int planeid = root["planeids"][i].asInt();
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid=\'%d\'",table_name_plane,planeid);
			if(queryRows(sqlbuffer,sqllen) <= 0){
				LOG(ERROR,"planeids is invalid:%s",sqlbuffer);
				backfailure(sock,root,type,"planeids is invalid");
				return false;
			}
		}
		planeids = writer.write(root["planeids"]);
		if(planeids[planeids.size()-1] == '\n')
			planeids[planeids.size()-1] = 0;
		
	}
	
/*	if(!root["planetaskinfo"].isNull()){
		if(!root["planetaskinfo"].isObject()){
			LOG(ERROR,"planetaskinfo is not object");
			backfailure(sock,root,type,"planetaskinfo is not object");
			return false;
		}
		planetaskinfo = writer.write(root["planetaskinfo"]);
		if(planetaskinfo[planetaskinfo.size()-1] == '\n')
			planetaskinfo[planetaskinfo.size()-1] = 0;
	}*/
	
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n')
			info[info.size()-1] = 0;
	}

	int taskid = getMaxTaskid(&sqlfd,companyid);
	if(taskid < 0){
		LOG(ERROR,"create taskid failed");
		backfailure(sock,root,type,"create taskid failed");
		return false;
	}
	
	++taskid;
	LOG(DEBUG,"taskid:%d",taskid);
	/*sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%d\',\'%s\',\'%s\',\'%s\',\'%s\')",
		table_name_task,taskid,companyid.c_str(),planeids.c_str(),planetaskinfo.c_str(),info.c_str());*/
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%d\',\'%s\',\'%s\',\'%s\')",
		table_name_task,taskid,companyid.c_str(),planeids.c_str(),info.c_str());
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR,"insert failed");
		backfailure(sock,root,type,"insert failed");
		return false;
	}
	LOG(DEBUG,"insert success");
	root["taskid"]=taskid;
	backsuccess(sock,root,type,"insert succcess");
	return true;	
}

bool SqlWork::RWSC(Json::Value&root,int sock)
{
	std::string type="RWSC";
	if(!root.isMember("taskid")){
		LOG(ERROR,"need taskid");
		backfailure(sock,root,type,"need taskid");
		return false;
	}

	if(!root["taskid"].isInt()){
		LOG(ERROR,"taskid is not int");
		backfailure(sock,root,type,"taskid is not int");
		return false;
	}
	
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}

	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	int taskid = root["taskid"].asInt();
	string companyid = root["companyid"].asString();
	LOG(DEBUG,"taskid:%d",taskid);
	LOG(DEBUG,"companyid:%s",companyid.c_str());

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where taskid=\'%d\' and companyid=\'%s\'",
	table_name_task,taskid,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of taskid for the companyid:%s",sqlbuffer);
		backfailure(sock,root,type,"not found record of taskid for the companyid");
		return false;
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where taskid=\'%d\' and companyid=\'%s\'",
	table_name_task,taskid,companyid.c_str());
	if(query(sqlbuffer, sqllen) == false){
		LOG(ERROR,"delete failed:%s",sqlbuffer);
		backfailure(sock,root,type,"delete failed");
		return false;
	}
	LOG(DEBUG,"delete success");
	backsuccess(sock, root,type,"delete success");
	return true;
}

bool SqlWork::RWXG(Json::Value&root,int sock)
{
	std::string type="RWXG";
	if(!root.isMember("taskid")){
		LOG(ERROR,"need taskid");
		backfailure(sock,root,type,"need taskid");
		return false;
	}

	if(!root["taskid"].isInt()){
		LOG(ERROR,"taskid is not int");
		backfailure(sock,root,type,"taskid is not int");
		return false;
	}
	
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}

	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	int taskid = root["taskid"].asInt();
	string companyid = root["companyid"].asString();
	LOG(DEBUG,"taskid:%d",taskid);
	LOG(DEBUG,"companyid:%s",companyid.c_str());

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where taskid=\'%d\' and companyid=\'%s\'",
	table_name_task,taskid,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of taskid for the companyid:%s",sqlbuffer);
		backfailure(sock,root,type,"not found record of taskid for the companyid");
		return false;
	}

	Json::FastWriter writer;
	int updateValue = 0;
	if(!root["planeids"].isNull()){
		if(!root["planeids"].isArray()){
			LOG(ERROR,"planeids is not array");
			backfailure(sock,root,type,"planeids is not array");
			return false;
		}
		for(int i =0;i<root["planeids"].size();++i){
			if(!root["planeids"][i].isInt()){
				LOG(ERROR,"planeid of planeids is not int");
				backfailure(sock,root,type,"planeid of planeids is not int");
				return false;
			}
			int planeid = root["planeids"][i].asInt();
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid=\'%d\'",table_name_plane,planeid);
			if(queryRows(sqlbuffer,sqllen) <= 0){
				LOG(ERROR,"planeids is invalid");
				backfailure(sock,root,type,"planeids is invalid");
				return false;
			}
		}
	
		std::string planeids = writer.write(root["planeids"]);
		if(planeids[planeids.size()-1] == '\n')
			planeids[planeids.size()-1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set planeids=\'%s\' where taskid=\'%d\' and companyid=\'%s\'",
			table_name_task,planeids.c_str(),taskid,companyid.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update planeids failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update planeids failed");
			return false;
		}
		++updateValue;
	}
	
//检查planetaskinfo并更新
/*	if(!root["planetaskinfo"].isNull()){
		if(!root["planetaskinfo"].isObject()){
			LOG(ERROR,"planetaskinfo is not object");
			backfailure(sock,root,type,"planetaskinfo is not object");
			return false;
		}
		std::string planetaskinfo = writer.write(root["planetaskinfo"]);
		if(planetaskinfo[planetaskinfo.size()-1] == '\n')
			planetaskinfo[planetaskinfo.size()-1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set planetaskinfo=\'%s\' where taskid=\'%d\' and companyid=\'%s\'",
		table_name_task,planetaskinfo.c_str(),taskid,companyid.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update planetaskinfo failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update planetaskinfo failed");
			return false;
		}
		++updateValue;
	}*/

//检查info并更新
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
	
		std::string info = writer.write(root["info"]);
		if(info[info.size() -1] = '\n')
			info[info.size() -1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set info=\'%s\' where taskid=\'%d\' and companyid=\'%s\'",
			table_name_task,info.c_str(),taskid,companyid.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update info failed:%s",sqlbuffer);
			backfailure(sock,root,type,"update info failed");
			return false;
		}
		+updateValue;
	}
	if(updateValue == 0){
		LOG(ERROR,"update failed");
		backfailure(sock,root,type,"update failed");
		return false;
	}
	LOG(DEBUG,"update success");
	backsuccess(sock,root,type,"update success");
	return true;
}

bool SqlWork::RWCX(Json::Value&root,int sock)
{
	std::string type="RWCX";
/*	if(!root.isMember("taskid")){
		LOG(ERROR,"need taskid");
		backfailure(sock,root,type,"need taskid");
		return false;
	}
*/
/*	if(!root["taskid"].isInt()){
		LOG(ERROR,"taskid is not int");
		backfailure(sock,root,type,"taskid is not int");
		return false;
	}*/
	
	if(!root.isMember("companyid")){
		LOG(ERROR,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}

	if(!root["companyid"].isString()){
		LOG(ERROR,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
//	int taskid = root["taskid"].asInt();
	string companyid = root["companyid"].asString();
//	LOG(DEBUG,"taskid:%d",taskid);
	LOG(DEBUG,"companyid:%s",companyid.c_str());

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"companyid is invalid:%s",sqlbuffer);
		backfailure(sock,root,type,"companyid is invalid");
		return false;
	}
	
/*	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where taskid=\'%d\' and companyid=\'%s\'",
	table_name_task,taskid,companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of taskid for the companyid:%s",sqlbuffer);
		backfailure(sock,root,type,"not found record of taskid for the companyid");
		return false;
	}*/

//	sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where taskid=\'%d\' and companyid=\'%s\'",
//	table_name_task,taskid,companyid.c_str());
	sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_task,companyid.c_str());
	MYSQL_RES *res = queryRes(sqlbuffer,sqllen);
	if(res == NULL){
		LOG(ERROR,"select failed:%s",sqlbuffer);
		backfailure(sock,root,type,"select failed");
		return false;
	}

	int sumrows = getRowNums(res);
	root["taskids"].resize(sumrows);
	root["planeids"].resize(sumrows);
//	root["planetaskinfos"].resize(sumrows);
	root["infos"].resize(sumrows);
	Json::Value planeidsValue;
	Json::Value planetaskinfoValue;
	Json::Value infoValue;
	MYSQL_ROW row;
	int index = 0;
	while(row = getRow(res))
	{
/*		if(row[0] && row[2] && row[3] && row[4]){
			if(string_to_json(row[2],planeidsValue) &&
				string_to_json(row[3],planetaskinfoValue)&&
				string_to_json(row[4],infoValue)){
				root["taskids"][index] = atoi(row[0]);
				root["planeids"][index] = planeidsValue;
				root["planetaskinfos"][index] = planetaskinfoValue;
				root["infos"][index] = infoValue;
				++index;
			}
		}
	*/
		if(row[0] && row[2] && row[3]){
			if(string_to_json(row[2],planeidsValue) &&
				string_to_json(row[3],infoValue)){
				root["taskids"][index] = atoi(row[0]);
				root["planeids"][index] = planeidsValue;
			//	root["planetaskinfos"][index] = planetaskinfoValue;
				root["infos"][index] = infoValue;
				++index;
			}
		}
	}
	result_free(res);
	LOG(DEBUG,"select success");
	backsuccess(sock,root,type,"select success");
	return true;
}

/*设备*/
bool SqlWork::SBZJ(Json::Value&root,int sock)
{
	LOG(DEBUG,"SBZJ:%s",root.toStyledString().c_str());

	std::string devid,info,companyid;
	int devtypeid=0,planeid=0,pilotid=0;
	std::string type = "SBZJ";
	if(!root.isMember("devid")){
		LOG(ERROR,"need devid");
		backfailure(sock, root, type, "need devid");
		return false;
	}
	if(!root.isMember("devtypeid")){
		LOG(ERROR,"need devtypeid");
		backfailure(sock, root, type, "need devtypeid");
		return false;
	}
	if(!root["devid"].isString()){
		LOG(ERROR,"devid is not string");
		backfailure(sock, root, type,"devid is not string");
		return false;
	}
	if(!root["devtypeid"].isInt()){
		LOG(ERROR,"devid is not int");
		backfailure(sock, root, type,"devid is not int");
		return false;
	}


	devid = root["devid"].asString();
	devtypeid = root["devtypeid"].asInt();

	LOG(DEBUG,"devid:%s",devid.c_str());
	LOG(DEBUG, "devtypeid:%d",devtypeid);

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devtypeid=\'%d\' and devid=\'%s\'",
		table_name_dev,devtypeid,devid.c_str());
	if(queryRows(sqlbuffer, sqllen) > 0){
		LOG(ERROR,"devid and devtypeid aleady existed");
		backfailure(sock,root,type,"devid and devtypeid aleady existed");
		return false;
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devtypeid=\'%d\'",table_name_devtype,devtypeid);
	if(queryRows(sqlbuffer, sqllen) <= 0){
		LOG(ERROR,"devtypeid is invalid");
		backfailure(sock, root, type,"devtypeid is invalid");
		return false;
	}
	
	if(!root["companyid"].isNull()){
		if(!root["companyid"].isString()){
			LOG(ERROR,"companyid is not string");
			backfailure(sock, root, type,"companyid is not string");
			return false;
		}
		companyid = root["companyid"].asString();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
		if(queryRows(sqlbuffer,sqllen) <= 0){
			LOG(ERROR,"companyid is invalid");
			backfailure(sock, root, type,"companyid is invalid");
			return false;
		}	
	}
	if(!root["planeid"].isNull()){
		if(!root["planeid"].isInt()){
			LOG(ERROR,"planeid is not int");
			backfailure(sock, root, type,"planeid is not int");
			return false;
		}
		planeid = root["planeid"].asInt();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid=\'%d\'",table_name_plane,planeid);
		if(queryRows(sqlbuffer,sqllen) <= 0){
			LOG(ERROR,"planeid is invalid");
			backfailure(sock, root, type,"planeid is invalid");
			return false;
		}	
	}

	if(!root["pilotid"].isNull()){
		if(!root["pilotid"].isInt()){
			LOG(ERROR,"pilotid is not int");
			backfailure(sock, root, type,"pilotid is not int");
			return false;
		}
		pilotid = root["pilotid"].asInt();
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where pilotid=\'%d\'",table_name_pilot,pilotid);
		if(queryRows(sqlbuffer,sqllen) <= 0){
			LOG(ERROR,"pilotid is invalid");
			backfailure(sock, root, type,"pilotid is invalid");
			return false;
		}
	}

	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock, root, type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n')
			info[info.size()-1] = 0;
	}
	//companyid是无效值，则planeid和pilotid也要设置为无效值
	if(companyid.size()==0){
		planeid = 0;
		pilotid = 0;
	}
	
	if(planeid > 0){
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select companyid from %s where planeid = \'%d\'",table_name_plane,planeid);
		MYSQL_RES *result = queryRes(sqlbuffer,sqllen);
		std::string planeCompanyid ="";
		if(result){
			MYSQL_ROW row = getRow(result);
			if(row && row[0]){
				planeCompanyid= row[0];
			}
			result_free(result);
		}
		if(planeCompanyid != companyid){
			LOG(ERROR,"companyid of plane not eq the companyid");
			backfailure(sock, root, type,"companyid of plane not eq the companyid");
			return false;
		}
	}
	
	if(pilotid > 0){
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select companyid from %s where pilotid = \'%d\'",table_name_pilot,pilotid);
		MYSQL_RES *result = queryRes(sqlbuffer,sqllen);
		std::string pilotCompanyid ="";
		if(result){
			MYSQL_ROW row = getRow(result);
			if(row && row[0]){
				pilotCompanyid= row[0];
			}
			result_free(result);
		}
		if(pilotCompanyid != companyid){
			LOG(ERROR,"companyid of pilot not eq the companyid");
			backfailure(sock, root, type,"companyid of pilot not eq the companyid");
			return false;
		}
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%s\',\'%d\',\'%s\',\'%d\',\'%d\',\'%s\')",table_name_dev,devid.c_str(),devtypeid,companyid.c_str(),planeid,pilotid,info.c_str());
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR,"insert failed");
		backfailure(sock, root, type,"insert failed");
		return false;
	}
	LOG(DEBUG,"insert dev success:%s",sqlbuffer);
	backsuccess(sock,root,type,"insert success");
	return true;
}


bool SqlWork::SBSC(Json::Value&root,int sock)
{
	std::string devid;
	int devtypeid;
	std::string type = "SBSC";
	if(!root.isMember("devid")){
		LOG(ERROR,"need devid");
		backfailure(sock, root, type, "need devid");
		return false;
	}
	if(!root.isMember("devtypeid")){
		LOG(ERROR,"need devtypeid");
		backfailure(sock, root, type, "need devtypeid");
		return false;
	}
	if(!root["devid"].isString()){
		LOG(ERROR,"devid is not string");
		backfailure(sock, root, type,"devid is not string");
		return false;
	}
	if(!root["devtypeid"].isInt()){
		LOG(ERROR,"devid is not int");
		backfailure(sock, root, type,"devid is not int");
		return false;
	}
	devid = root["devid"].asString();
	devtypeid = root["devtypeid"].asInt();

	LOG(DEBUG,"devid:%s",devid.c_str());
	LOG(DEBUG, "devtypeid:%d",devtypeid);

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devid=\'%s\' and devtypeid=\'%d\'",
		table_name_dev,devid.c_str(),devtypeid);
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of devtypeid for devid");
		backfailure(sock, root, type,"not found record of devtypeid for devid");
		return false;
	}
	
	sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where devid=\'%s\' and devtypeid=\'%d\'",
	table_name_dev,devid.c_str(),devtypeid);
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR,"delete failed");
		backfailure(sock, root, type,"delete failed");
		return false;
	}
	LOG(DEBUG,"delete success");
	backsuccess(sock, root,type,"delete success");
	return true;
}

bool sbxg_check_companyid_of_plane(MYSQL &sqlfd,std::string &companyid,std::string &devid,int devtypeid)
{
	const int bufferlen = 1024;
	char *sqlbuffer = new char[1024];
	int sqllen = snprintf(sqlbuffer,bufferlen,"select planeid from %s where devid=\'%s\' and devtypeid=%d",table_name_dev,devid.c_str(),devtypeid);
	MYSQL_RES *res  = query_res(sqlfd,sqlbuffer,sqllen);
	int planeid = 0;
	if(res){
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0])
		planeid = atoi(row[0]);
		mysql_free_result(res);
	}
	if(planeid == 0){ //planeid是null
		delete[]sqlbuffer;
		return true;
	}
	
	sqllen = snprintf(sqlbuffer,bufferlen,"select companyid from %s where planeid=%d",table_name_plane,planeid);
	res = query_res(sqlfd,sqlbuffer,sqllen);
	std::string planeCompanyid;
	if(res){
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0])
			planeCompanyid = row[0];
		mysql_free_result(res);
	}
	delete[]sqlbuffer;
	if(companyid == planeCompanyid)
		return true;
	return false;
}


bool sbxg_check_companyid_of_pilot(MYSQL &sqlfd,std::string &companyid,std::string &devid,int devtypeid)
{
	const int bufferlen = 1024;
	char *sqlbuffer = new char[1024];
	int sqllen = snprintf(sqlbuffer,bufferlen,"select pilotid from %s where devid=\'%s\' and devtypeid=%d",table_name_dev,devid.c_str(),devtypeid);
	MYSQL_RES *res  = query_res(sqlfd,sqlbuffer,sqllen);
	int pilotid = 0;
	if(res){
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0])
		pilotid = atoi(row[0]);
		mysql_free_result(res);
	}
	if(pilotid == 0){ //planeid是null
		delete[]sqlbuffer;
		return true;
	}
	
	sqllen = snprintf(sqlbuffer,bufferlen,"select companyid from %s where pilotid=%d",table_name_pilot,pilotid);
	res = query_res(sqlfd,sqlbuffer,sqllen);
	std::string pilotCompanyid;
	if(res){
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0])
			pilotCompanyid = row[0];
		mysql_free_result(res);
	}
	delete[]sqlbuffer;
	if(companyid == pilotCompanyid)
		return true;
	return false;
}

bool sbxg_plane_check_companyid(MYSQL &sqlfd,int planeid,std::string &devid,int devtypeid)
{
	const int bufferlen = 1024;
	char *sqlbuffer = new char[1024];
	if(!sqlbuffer){
		LOG(DEBUG, "new sqlbuffer err");
		return false;
	}
	int sqllen = snprintf(sqlbuffer,bufferlen,"select companyid from %s where devid=\'%s\' and devtypeid=%d",\
		table_name_dev,devid.c_str(),devtypeid);
	MYSQL_RES *res  = query_res(sqlfd,sqlbuffer,sqllen);
	std::string companyid;
	if(res){

		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0])
		companyid = row[0];
		mysql_free_result(res);
		res = NULL;
	}

	if(companyid.size()==0){ //companyid 为 null的时候可以直接修改planeid，所以反馈true
		delete[]sqlbuffer;
		return true;
	}
	sqllen = snprintf(sqlbuffer,bufferlen,"select companyid from %s where planeid=%d",table_name_plane,planeid);
	res = query_res(sqlfd,sqlbuffer,sqllen);
	LOG(DEBUG,"sqlbuffer:%s",sqlbuffer);
	std::string planeCompanyid;
	if(res){
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0])
			planeCompanyid = row[0];
		mysql_free_result(res);
	}
	delete[]sqlbuffer;
	if(companyid == planeCompanyid)
		return true;
	return false;
	
}

bool sbxg_pilot_check_companyid(MYSQL &sqlfd,int pilotid,std::string &devid,int devtypeid)
{
	const int bufferlen = 1024;
	char *sqlbuffer = new char[1024];
	int sqllen = snprintf(sqlbuffer,bufferlen,"select companyid from %s where devid=\'%s\' and devtypeid=%d",\
		table_name_dev,devid.c_str(),devtypeid);
	MYSQL_RES *res  = query_res(sqlfd,sqlbuffer,sqllen);
	std::string companyid;
	if(res){
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0])
		companyid = row[0];
		mysql_free_result(res);
		res = NULL;
	}
	if(companyid.size()==0){ //companyid 为 null的时候可以直接修改planeid，所以反馈true
		delete[]sqlbuffer;
		return true;
	}
	
	sqllen = snprintf(sqlbuffer,bufferlen,"select companyid from %s where pilotid=%d",table_name_pilot,pilotid);
	res = query_res(sqlfd,sqlbuffer,sqllen);
	std::string pilotidCompanyid;
	if(res){
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0])
			pilotidCompanyid = row[0];
		mysql_free_result(res);
	}
	delete[]sqlbuffer;
	if(companyid == pilotidCompanyid)
		return true;
	return false;
}




bool SqlWork::SBXG(Json::Value&root,int sock)
{
	std::string devid;
	int devtypeid;
	std::string type = "SBXG";
	if(!root.isMember("devid")){
		LOG(ERROR,"need devid");
		backfailure(sock, root, type, "need devid");
		return false;
	}
	if(!root.isMember("devtypeid")){
		LOG(ERROR,"need devtypeid");
		backfailure(sock, root, type, "need devtypeid");
		return false;
	}
	if(!root["devid"].isString()){
		LOG(ERROR,"devid is not string");
		backfailure(sock, root, type,"devid is not string");
		return false;
	}
	if(!root["devtypeid"].isInt()){
		LOG(ERROR,"devid is not int");
		backfailure(sock, root, type,"devid is not int");
		return false;
	}
	devid = root["devid"].asString();
	devtypeid = root["devtypeid"].asInt();

	LOG(DEBUG,"devid:%s",devid.c_str());
	LOG(DEBUG, "devtypeid:%d",devtypeid);

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devid=\'%s\' and devtypeid=\'%d\'",
		table_name_dev,devid.c_str(),devtypeid);
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of devtypeid for devid");
		backfailure(sock, root, type,"not found record of devtypeid for devid");
		return false;
	}

	int updateValue = 0;	
	if(root.isMember("planeid")){
		if(!root["planeid"].isInt()){
			LOG(ERROR,"planeid is not int");
			backfailure(sock, root, type,"planeid is not int");
			return false;
		}
		
		int planeid = root["planeid"].asInt();
		LOG(DEBUG,"planeid:%d",planeid);

		if(sbxg_plane_check_companyid(sqlfd,planeid,devid, devtypeid) == false){
			LOG(ERROR,"companyid of plane not eq the companyid of dev");
			backfailure(sock, root, type,"companyid of plane not eq the companyid of dev");
			return false;
		}
		
		if(planeid < 0){
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set planeid=null where devid=\'%s\' and devtypeid=\'\%d'",
			table_name_dev,devid.c_str(),devtypeid);
		}else{
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where planeid=\'%d\'",table_name_plane,planeid);
			if(queryRows(sqlbuffer,sqllen) <= 0){
				LOG(ERROR,"planeid is invalid");
				backfailure(sock, root, type,"planeid is invalid");
				return false;
			}
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set planeid=\'%d\' where devid=\'%s\' and devtypeid=\'\%d'",
				table_name_dev,planeid,devid.c_str(),devtypeid);
		}
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update planeid failed");
			backfailure(sock, root, type,"update planeid failed");
			return false;
		}
		++updateValue;
	}
//修改pilotid
	if(root.isMember("pilotid")){
		if(!root["pilotid"].isInt()){
			LOG(ERROR,"pilotid is not int");
			backfailure(sock, root, type,"pilotid is not int");
			return false;	
		}
		int pilotid = root["pilotid"].asInt();
		LOG(DEBUG,"pilotid:%d",pilotid);
		if(sbxg_pilot_check_companyid(sqlfd,pilotid, devid,devtypeid) == false){
			LOG(ERROR,"companyid of pilot not eq companyid of dev");
			backfailure(sock, root, type,"companyid of pilot not eq companyid of dev");
			return false;
		}
		
		if(pilotid <0){
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set pilotid=null where devid=\'%s\' and devtypeid=\'\%d'",
			table_name_dev,devid.c_str(),devtypeid);
		}else{
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where pilotid=\'%d\'",table_name_pilot,pilotid);
			if(queryRows(sqlbuffer,sqllen) <= 0){
				LOG(ERROR,"pilotid is invalid");
				backfailure(sock, root, type,"pilotid is invalid");
				return false;	
			}
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set pilotid=\'%d\' where devid=\'%s\' and devtypeid=\'\%d'",
			table_name_dev,pilotid,devid.c_str(),devtypeid);
		}

		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update pilotid failed");
			backfailure(sock, root, type,"update pilotid failed");
			return false;
		}
		++updateValue;
	}

	if(root.isMember("companyid")){
		if(!root["companyid"].isString()){
			LOG(ERROR,"companyid is not string");
			backfailure(sock, root, type,"companyid is not stroing");
			return false;	
		}

		std::string companyid = root["companyid"].asString();
		LOG(DEBUG,"companyid:%s",companyid.c_str());
		
		//检查公司id是否存在
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where companyid=\'%s\'",table_name_company,companyid.c_str());
		if(queryRows(sqlbuffer,sqllen) <= 0){
			LOG(ERROR,"companyid is invalid");
			backfailure(sock, root, type,"companyid is invalid");
			return false;
		}
		if(sbxg_check_companyid_of_plane(sqlfd,companyid,devid,devtypeid) == false){
			LOG(ERROR,"这个公司号和飞机对应的公司编号不一致");
			backfailure(sock, root, type,"comapnyid not eq companyid of plane");
			return false;
		}
		if(sbxg_check_companyid_of_pilot(sqlfd,companyid,devid,devtypeid) == false){
			LOG(ERROR,"这个公司号和飞行员对应的公司编号不一致");
			backfailure(sock, root, type,"comapnyid not eq companyid of pilotid");
			return false;
		}
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set companyid =\'%s\' where devid=\'%s\' and devtypeid=\'\%d'",
			table_name_dev,companyid.c_str(),devid.c_str(),devtypeid);
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update companyid failed");
			backfailure(sock, root, type,"update companyid failed");
			return false;
		}
		++updateValue;		
	}

//修改info
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock, root, type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		std::string info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n')
			info[info.size()-1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set info =\'%s\' where devid=\'%s\' and devtypeid=\'\%d'",
			table_name_dev,info.c_str(),devid.c_str(),devtypeid);
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update info failed");
			backfailure(sock, root, type,"update info failed");
			return false;
		}
		++updateValue;
	}
	if(updateValue){
		Json::Value infoobject;
		root["planeid"]=0;
		root["pilotid"]=0;
		root["companyid"]="";
		root["info"]=infoobject;
		int nlen = snprintf(sqlbuffer,max_sql_buffer_len,"select companyid,planeid,pilotid,info from %s where devid=\'%s\' and devtypeid=%d",\
		table_name_dev,devid.c_str(),devtypeid);
		MYSQL_RES *result = queryRes(sqlbuffer, nlen);
		if(result){
			MYSQL_ROW row = getRow(result);
			if(row){
				if(row[0])
					root["companyid"] = row[0];
				if(row[1])
					root["planeid"] = atoi(row[1]);
				if(row[2])
					root["pilotid"] = atoi(row[2]);
				if(row[3] && string_to_json(row[3],infoobject))
					root["info"] = infoobject;
			}
			result_free(result);
		}
		LOG(DEBUG,"update success");
		backsuccess(sock, root,type,"update success");
		return true;
	}
	LOG(ERROR,"update failed");
	backfailure(sock, root, type,"update failed");
	return false;
}

bool SqlWork::SBCX(Json::Value&root,int sock)
{
	std::string devid;
	int devtypeid;
	std::string type = "SBCX";
	if(!root.isMember("devid")){
		LOG(ERROR,"need devid");
		backfailure(sock, root, type, "need devid");
		return false;
	}
	if(!root.isMember("devtypeid")){
		LOG(ERROR,"need devtypeid");
		backfailure(sock, root, type, "need devtypeid");
		return false;
	}
	if(!root["devid"].isString()){
		LOG(ERROR,"devid is not string");
		backfailure(sock, root, type,"devid is not string");
		return false;
	}
	if(!root["devtypeid"].isInt()){
		LOG(ERROR,"devid is not int");
		backfailure(sock, root, type,"devid is not int");
		return false;
	}
	devid = root["devid"].asString();
	devtypeid = root["devtypeid"].asInt();

	LOG(DEBUG,"devid:%s",devid.c_str());
	LOG(DEBUG, "devtypeid:%d",devtypeid);

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devid=\'%s\' and devtypeid=\'%d\'",
		table_name_dev,devid.c_str(),devtypeid);
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of devtypeid for devid");
		backfailure(sock, root, type,"not found record of devtypeid for devid");
		return false;
	}
	
	MYSQL_RES *res = queryRes(sqlbuffer,sqllen);
	MYSQL_ROW row;
	Json::Value val;	
	if(res && (row=getRow(res)) && getFields(res)>= 5){
		root["planeid"] = 0;
		root["pilotid"] = 0;
		root["companyid"]="";
		root["info"] = val;
		if(row[2]){
			root["companyid"] = row[2];
		}
		if(row[3])
			root["planeid"] = atoi(row[3]);
		if(row[4])
			root["pilotid"] = atoi(row[4]);
		if(row[5] && string_to_json(row[5],val))
			root["info"] = val;
		LOG(DEBUG,"select success");
		backsuccess(sock,root,type,"selected");
		result_free(res);
		return true;
	}
	result_free(res);
	LOG(ERROR,"select failed");
	backfailure(sock, root, type,"select failed");
	return false;
}


bool SqlWork::SJYZJ(Json::Value&root,int sock)
{
	string ipaddr,info;
	int devtypeid = 0;
	std::string type="SJYZJ";
	if(!root.isMember("ipaddr")){
		LOG(ERROR,"need ipaddr");
		backfailure(sock,root,type,"need ipaddr");
		return false;
	}

	if(!root["ipaddr"].isString()){
		LOG(ERROR,"ipaddr is not string");
		backfailure(sock,root,type,"ipaddr is not string");
		return false;
	}

	ipaddr = root["ipaddr"].asString();
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where ipaddr=\'%s\'",table_name_datasrc,ipaddr.c_str());
	if(queryRows(sqlbuffer,sqllen) > 0){
		LOG(ERROR,"ipaddr already existed");
		backfailure(sock,root,type,"ipaddr already existed");
		return false;
	}
	
	if(!root["devtypeid"].isNull()){
		if(!root["devtypeid"].isInt()){
			LOG(ERROR,"devtypeid is not int");
			backfailure(sock,root,type,"devtypeid is not int");
			return false;
		}
		devtypeid = root["devtypeid"].asInt();
		sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devtypeid=\'%d\'",table_name_devtype,devtypeid);
		if(queryRows(sqlbuffer,sqllen) <= 0){
			LOG(ERROR,"devtypeid is invalid");
			backfailure(sock,root,type,"devtypeid is invalid");
			return false;
		}
	}
	
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		info = writer.write(root["info"]);
		if(info[info.size()-1]=='\n')
			info[info.size()-1] = 0;
	}	

	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%s\',\'%d\',\'%s\')",
	table_name_datasrc,ipaddr.c_str(),devtypeid,info.c_str());
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR,"insert failed");
		backfailure(sock,root,type,"insert failed");
		return false;
	}
	LOG(DEBUG,"insert success");
	backsuccess(sock,root,type,"insert success");
	return true;	
}
	
bool SqlWork::SJYSC(Json::Value&root,int sock)
{
	string ipaddr;
	std::string type="SJYSC";
	if(!root.isMember("ipaddr")){
		LOG(ERROR,"need ipaddr");
		backfailure(sock,root,type,"need ipaddr");
		return false;
	}

	if(!root["ipaddr"].isString()){
		LOG(ERROR,"ipaddr is not string");
		backfailure(sock,root,type,"ipaddr is not string");
		return false;
	}
	ipaddr = root["ipaddr"].asString();

	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where ipaddr=\'%s\'",table_name_datasrc,ipaddr.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the ipaddr");
		backfailure(sock,root,type,"not found record of the ipaddr");
		return false;
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where ipaddr=\'%s\'",table_name_datasrc,ipaddr.c_str());
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR,"delete failed:%s",sqlbuffer);
		backfailure(sock,root,type,"delete failed");
		return false;
	}
	LOG(DEBUG,"delete success:%s",sqlbuffer);
	backsuccess(sock, root,type,"delete success");
	return true;
}

bool SqlWork::SJYXG(Json::Value&root,int sock)
{
	std::string type = "SJYXG";
	if(!root.isMember("ipaddr")){
		LOG(ERROR,"need ipaddr");
		backfailure(sock,root,type,"need ipaddr");
		return false;
	}
	if(!root["ipaddr"].isString()){
		LOG(ERROR,"ipaddr is not string");
		backfailure(sock,root,type,"ipaddr is not string");
		return false;
	}
	std::string ipaddr = root["ipaddr"].asString();
//检查是否存在对应的记录
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where ipaddr=\'%s\'",table_name_datasrc,ipaddr.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the ipaddr");
		backfailure(sock,root,type,"not found record of the ipaddr");
		return false;
	}

	int updateValue = 0;
	
//更新devtypeid	，需要检查有效性	
	if(!root["devtypeid"].isNull()){
		if(!root["devtypeid"].isInt()){
			LOG(ERROR,"devtypeid is not int");
			backfailure(sock,root,type,"devtypeid is not int");
			return false;
		}
		int devtypeid = root["devtypeid"].asInt();
		if(devtypeid < 0){
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set devtypeid=null where ipaddr=\'%s\'",
			table_name_datasrc,ipaddr.c_str());
		}
		else{
			sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devtypeid=\'%d\'",table_name_devtype,devtypeid);
			if(queryRows(sqlbuffer,sqllen) <= 0){
				LOG(ERROR,"devtypeid is invalid");
				backfailure(sock,root,type,"devtypeid is invalid");
				return false;
			}
			sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set devtypeid=\'%d\' where ipaddr=\'%s\'",
			table_name_datasrc,devtypeid,ipaddr.c_str());
		}
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update devtypeid failed");
			backfailure(sock,root,type,"update devtypeid failed");
			return false;
		}
		++updateValue; 
	}	
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		std::string	info = writer.write(root["info"]);
		if(info[info.size()-1]=='\n')
			info[info.size()-1] = 0;
		sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set info=\'%s\' where ipaddr=\'%s\'",
		table_name_datasrc,info.c_str(),ipaddr.c_str());
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update info failed");
			backfailure(sock,root,type,"update info failed");
			return false;
		}
		++updateValue; 
	}
	
	if(updateValue){
		LOG(DEBUG,"update success");
		backsuccess(sock,root,type,"update success");
		return true;
	}
	
	LOG(ERROR,"update failed");
	backfailure(sock,root,type,"update failed");
	return false;
}


bool SqlWork::SJYCX(Json::Value&root,int sock)
{
	std::string type = "SJYCX";
	if(!root.isMember("ipaddr")){
		LOG(ERROR,"need ipaddr");
		backfailure(sock,root,type,"need ipaddr");
		return false;
	}
	if(!root["ipaddr"].isString()){
		LOG(ERROR,"ipaddr is not string");
		backfailure(sock,root,type,"ipaddr is not string");
		return false;
	}
	std::string ipaddr = root["ipaddr"].asString();
	//检查是否存在对应的记录
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where ipaddr=\'%s\'",table_name_datasrc,ipaddr.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found record of the ipaddr");
		backfailure(sock,root,type,"not found record of the ipaddr");
		return false;
	}
	
	MYSQL_RES *res = queryRes(sqlbuffer,sqllen);
	MYSQL_ROW row;
	if(res && (row=getRow(res)) && getFields(res)>=3){
		Json::Value val;
		root["devtypeid"] = 0;
		root["info"] =val;
		if(row[1]){
			root["devtypeid"] = atoi(row[1]);
		}
		if(row[2] && string_to_json(row[2],val)){
			root["info"] =val;
		}
		LOG(DEBUG,"select success:%s",sqlbuffer);
		backsuccess(sock,root,type,"select success");
		result_free(res);
		return true;
	}
	result_free(res);
	LOG(ERROR,"select failed:%s",sqlbuffer);
	backfailure(sock,root,type,"select failed");
	return false;
}

static int getMaxDevtypeId(MYSQL *sqlfd)
{
	char buffer[1024];
	int maxidnum=0;
	MYSQL_RES *res = NULL;
	int sqllen=snprintf(buffer,1024,"select max(devtypeid) from %s",table_name_devtype);
	if(mysql_real_query(sqlfd,buffer,sqllen)){
		LOG(ERROR,"query failed:%s,%s",buffer,mysql_error(sqlfd));
		return -1;
	}
	res = mysql_store_result(sqlfd);
	if(res == NULL){
		LOG(ERROR,"result failed:%s,%s",buffer,mysql_error(sqlfd));
		return -1;
	}
	MYSQL_ROW row;
	if((row=mysql_fetch_row(res)) && row[0]){
		maxidnum = atoi(row[0]);
	}
	if(res)
		mysql_free_result(res);
	return maxidnum;
}

bool SqlWork::SBLXZJ(Json::Value&root,int sock)
{	
	std::string ipaddrlist,info;
	Json::FastWriter writer;
	std::string type="SBLXZJ";
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n')
			info[info.size()-1] = 0;
	}
	
/*	if(root.isMember("ipaddrlist") && root["ipaddrlist"].isArray()){
		ipaddrlist= writer.write(root["ipaddrlist"]);
//检查设备类型列表是否被允许
		for(int i=0;i<root["ipaddrlist"].size();++i){
			if(root["ipaddrlist"][i].isString()){
				string ipaddr = root["ipaddrlist"][i].asString();
				int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where ipaddr=\'%s\'",table_name_datasrc,ipaddr.c_str());
				if(queryRows(sqlbuffer,sqllen) <= 0){
					root["result"]["value"] = "no";
					root["result"]["reason"] = "ipaddr of ipaddrlist is invalid";
					reval["content"] = root;
					sendJson(sock,reval);
					LOG(DEBUG,"ipaddr of ipaddrlist is invalid");
					return false;	
				}
			}
			else{
				root["result"]["value"] = "no";
				root["result"]["reason"] = "ipaddrlist is invalid";
				reval["content"] = root;
				sendJson(sock,reval);
				LOG(DEBUG,"ipaddrlist is invalid");
				return false;
			}
		}
	}
*/

	int devtypeid = getMaxDevtypeId(&sqlfd);
	if(devtypeid <0){
		LOG(ERROR,"create devtypeid is failed");
		backfailure(sock,root,type,"create devtypeid is failed");
		return false;
	}
	++devtypeid;
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"insert into %s values(\'%d\',\'%s\')",table_name_devtype,devtypeid,info.c_str());
	if(query(sqlbuffer, sqllen) == false){
		LOG(ERROR,"insert failed");
		backfailure(sock,root,type,"insert failed");
		return false;
	}
	LOG(DEBUG,"insert success");
	root["devtypeid"]=devtypeid;
	backsuccess(sock,root,type,"insert success");
	return true;
}

bool update_datasrc_devtypeid(MYSQL sqlfd,int sock,int devtypeid,SqlWork &sqlwork)
{
	const int maxbufferlen = 1024;
	char *querybuffer = new char[maxbufferlen];
	if(!querybuffer){
		LOG(ERROR,"new err");
		return false;
	}
	int querylen = snprintf(querybuffer,maxbufferlen,"select ipaddr from %s where devtypeid=%d",table_name_datasrc,devtypeid);
	MYSQL_RES *res = query_res(sqlfd,querybuffer,querylen);
	MYSQL_ROW row = NULL;
	if(res == NULL){
		LOG(ERROR,"query result err:%s,%s",querybuffer,mysql_error(&sqlfd));
		goto ERR;
	}

	while((row = mysql_fetch_row(res)) && row[0]){

		snprintf(querybuffer,maxbufferlen,"update %s set devtypeid=%d where ipaddr=\'%s\'",table_name_datasrc,devtypeid,row[0]);
		
		Json::Value content;
		content["ipaddr"]=row[0];
		content["devtypeid"] = -1;
		if(!sqlwork.SJYXG(content,sock)){
			goto ERR;
		}
	}
	mysql_free_result(res);
	delete[]querybuffer;
	return true;
ERR:
	if(res)
		mysql_free_result(res);
	delete[]querybuffer;
	return false;
}

bool delete_dev_devtypeid(MYSQL sqlfd,int sock,int devtypeid,SqlWork &sqlwork)
{
	const int maxbufferlen = 1024;
	char *querybuffer = new char[maxbufferlen];
	if(!querybuffer){
		LOG(ERROR,"new err");
		return false;
	}
	int querylen = snprintf(querybuffer,maxbufferlen,"select devid from %s where devtypeid=%d",table_name_dev,devtypeid);
	MYSQL_RES *res = query_res(sqlfd,querybuffer,querylen);
	MYSQL_ROW row = NULL;
	if(res == NULL){
		LOG(ERROR,"query result err:%s,%s",querybuffer,mysql_error(&sqlfd));
		goto ERR;
	}
	while((row = mysql_fetch_row(res)) && row[0]){
		Json::Value content;
		content["devid"]=row[0];
		content["devtypeid"]=devtypeid;
		if(!sqlwork.SBSC(content,sock)){
			goto ERR;
		}
	}
	mysql_free_result(res);
	delete[]querybuffer;
	return true;
ERR:
	if(res)
		mysql_free_result(res);
	delete[]querybuffer;
	return false;
}

bool SqlWork::SBLXSC(Json::Value&root,int sock)
{
	int devtypeid = 0;
	std::string type = "SBLXSC";
	if(!root.isMember("devtypeid")){
		LOG(ERROR,"need devtypeid");
		backfailure(sock,root,type,"need devtypeid");
		return false;
	}
	if(!root["devtypeid"].isInt()){
		LOG(ERROR,"devtypeid is not int");
		backfailure(sock,root,type,"devtypeid is not int");
		return false;
	}

	devtypeid = root["devtypeid"].asInt();
	LOG(DEBUG,"devtypeid:%d",devtypeid);
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devtypeid=\'%d\'",table_name_devtype,devtypeid);
#if 0
	if(queryRows(sqlbuffer, sqllen) <= 0){
		LOG(ERROR,"not found record of the devtypeid");
		backfailure(sock,root,type,"not found record of the devtypeid");
		return false;
	}
#endif
	update_datasrc_devtypeid(sqlfd,sock,devtypeid,*this);
	
	delete_dev_devtypeid(sqlfd, sock, devtypeid, *this);

	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"delete from %s where devtypeid=\'%d\'",table_name_devtype,devtypeid);
	if(query(sqlbuffer,sqllen) == false){
		LOG(ERROR,"delete failed");
		backfailure(sock,root,type,"delete failed");
		return false;
	}
	LOG(DEBUG,"delete success");
	backsuccess(sock,root,type,"deleted");
	return true;
}


bool SqlWork::SBLXXG(Json::Value&root,int sock)
{
	int devtypeid = 0;
	std::string type = "SBLXXG";
	if(!root.isMember("devtypeid")){
		LOG(ERROR,"need devtypeid");
		backfailure(sock,root,type,"need devtypeid");
		return false;
	}
	if(!root["devtypeid"].isInt()){
		LOG(ERROR,"devtypeid is not int");
		backfailure(sock,root,type,"devtypeid is not int");
		return false;
	}

	devtypeid = root["devtypeid"].asInt();
	LOG(DEBUG,"devtypeid:%d",devtypeid);
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devtypeid=\'%d\'",table_name_devtype,devtypeid);
	if(queryRows(sqlbuffer, sqllen) <= 0){
		LOG(ERROR,"not found record of the devtypeid");
		backfailure(sock,root,type,"not found record of the devtypeid");
		return false;
	}

/*
	if(root.isMember("ipaddrlist") && root["ipaddrlist"].isArray()){
		ipaddrlist= writer.write(root["ipaddrlist"]);
		for(int i=0;i<root["ipaddrlist"].size();++i){
			if(root["ipaddrlist"][i].isString()){
				string ipaddr = root["ipaddrlist"][i].asString();
				int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where ipaddr=\'%s\'",table_name_datasrc,ipaddr.c_str());
				if(queryRows(sqlbuffer,sqllen) <= 0){
					root["result"]["value"] = "no";
					root["result"]["reason"] = "ipaddr of ipaddrlist is invalid";
					reval["content"] = root;
					sendJson(sock,reval);
					LOG(DEBUG,"ipaddr of ipaddrlist is invalid");
					return false;	
				}
			}
			else{
				root["result"]["value"] = "no";
				root["result"]["reason"] = "ipaddrlist is invalid";
				reval["content"] = root;
				sendJson(sock,reval);
				LOG(DEBUG,"ipaddrlist is invalid");
				return false;
			}
		}
		int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set ipaddrlist=\'%s\' where devtypeid=\'%d\'",
		table_name_devtype,ipaddrlist.c_str(),devtypeid);
		if(query(sqlbuffer,sqllen) == false){
			root["result"]["value"] = "no";
			root["result"]["reason"] = "update ipaddrlist failed";
			reval["content"] = root;
			sendJson(sock,reval);
			LOG(DEBUG,"update ipaddrlist failed");
			return false;
		}
		++updatenums;
	}
*/	
	int updateValue=0;
	if(!root["info"].isNull()){
		if(!root["info"].isObject()){
			LOG(ERROR,"info is not object");
			backfailure(sock,root,type,"info is not object");
			return false;
		}
		Json::FastWriter writer;
		std::string info = writer.write(root["info"]);
		if(info[info.size()-1] == '\n')
			info[info.size()-1] = 0;
		int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"update %s set info=\'%s\' where devtypeid=\'%d\'",
		table_name_devtype,info.c_str(),devtypeid);
		if(query(sqlbuffer,sqllen) == false){
			LOG(ERROR,"update info is success");
			backfailure(sock,root,type,"update info success");
			return false;
		}
		++updateValue;
	}
	if(updateValue){
		LOG(DEBUG,"update success");
		backsuccess(sock,root,type,"update success");
		return true;
	}
	LOG(ERROR,"update failed");
	backfailure(sock,root,type,"update failed");
	return false;
}

bool SqlWork::SBLXCX(Json::Value&root,int sock)
{
	int devtypeid = 0;
	std::string type = "SBLXCX";
	if(!root.isMember("devtypeid")){
		LOG(ERROR,"need devtypeid");
		backfailure(sock,root,type,"need devtypeid");
		return false;
	}
	if(!root["devtypeid"].isInt()){
		LOG(ERROR,"devtypeid is not int");
		backfailure(sock,root,type,"devtypeid is not int");
		return false;
	}

	devtypeid = root["devtypeid"].asInt();
	LOG(DEBUG,"devtypeid:%d",devtypeid);
	int sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select * from %s where devtypeid=\'%d\'",table_name_devtype,devtypeid);
	if(queryRows(sqlbuffer, sqllen) <= 0){
		LOG(ERROR,"not found record of the devtypeid");
		backfailure(sock,root,type,"not found record of the devtypeid");
		return false;
	}
	
	sqllen = snprintf(sqlbuffer,max_sql_buffer_len,"select *from %s where devtypeid=\'%d\'",table_name_devtype,devtypeid);
	MYSQL_RES *res = queryRes(sqlbuffer, sqllen);
	MYSQL_ROW row;
	if(res && (row=getRow(res)) && getFields(res)>=2){
		Json::Value info;
		root["info"]=info;
		if(row[1] && string_to_json(row[1],info))
			root["info"]=info;
		LOG(DEBUG,"select success");
		backsuccess(sock,root,type,"select success");	
		result_free(res);
		return true;
	}
	result_free(res);
	LOG(ERROR,"select failed:%s",sqlbuffer);
	backfailure(sock,root,type,"select failed");
	return false;
}


bool SqlWork::MMCX(Json::Value&root,int sock)
{
	string userid,companyid;
	std::string type ="MMCX";
	if(!root.isMember("userid")){
		LOG(DEBUG,"need userid");
		backfailure(sock,root,type,"need userid");
		return false;
	}
	if(!root.isMember("companyid")){
		LOG(DEBUG,"need companyid");
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	
	if(!root["userid"].isString()){
		LOG(DEBUG,"userid is not string");
		backfailure(sock,root,type,"userid is not string");
		return false;
	}
	
	if(!root["companyid"].isString()){
		LOG(DEBUG,"companyid is not string");
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	userid = root["userid"].asString();
	companyid = root["companyid"].asString();	
	LOG(DEBUG,"userid:%s",userid.c_str());
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	int sqllen =snprintf(sqlbuffer,max_sql_buffer_len,"select password from %s where userid=\'%s\' and companyid=\'%s\'",
	table_name_user,userid.c_str(),companyid.c_str());
	if(queryRows(sqlbuffer,sqllen) <= 0){
		LOG(ERROR,"not found:%s",sqlbuffer);
		backfailure(sock,root,type,"not found password");
		return false;
	}
	MYSQL_RES *res = queryRes(sqlbuffer,sqllen);
	MYSQL_ROW row;
	if(res &&(row=getRow(res))&&row[0]){
		root["password"] = row[0];
		LOG(DEBUG, "select success");
		backsuccess(sock,root,type,"select success");
		result_free(res);
		return true;
	}
	LOG(ERROR,"select failed:%s",sqlbuffer);
	backfailure(sock,root,type,"select failed");
	result_free(res);
	return false;
}
//| devid     | varchar(256) | NO   | PRI | NULL    |       |
//| devtypeid | int(11)      | NO   | PRI | NULL    |       |
//| planeid   | int(11)      | YES  |     | NULL    |       |
//| pilotid   | int(11)      | YES  |     | NULL    |       |
//| info      | text         | YES  |     | NULL    |       |

bool SqlWork::SBLB(Json::Value&root,int sock)
{
	int sqlLen = snprintf(sqlbuffer,max_sql_buffer_len,"select devid,devtypeid,companyid,planeid,pilotid,info from %s",table_name_dev);
	if(sqlLen < 0){
		LOG(ERROR,"snprintf:%s",strerror(errno));
		return false;
	}
	std::string type="SBLB";
	root["devids"].resize(0);
	root["devtypeids"].resize(0);
	root["companyids"].resize(0);
	root["planeids"].resize(0);
	root["pilotids"].resize(0);
	root["infos"].resize(0);
	MYSQL_RES * res =  queryRes(sqlbuffer, sqlLen);
	if(res == NULL){
		backfailure(sock,root,type,"not found records");
		return false;
	}
	int nums = 0;
	if((nums=getRowNums(res)) <= 0){
		LOG(DEBUG,"select records is %d",nums);
	//	backfailure(sock,root,type,"found records zero");
	//	return false;
	}
	
	root["devids"].resize(nums);
	root["devtypeids"].resize(nums);
	root["companyids"].resize(nums);
	root["planeids"].resize(nums);
	root["pilotids"].resize(nums);
	root["infos"].resize(nums);
	MYSQL_ROW row;
	int index=0;
	while((row=getRow(res)) && row[0] && row[1]){ //row[0] ,row[1]对应的是主键
		Json::Value info;
		root["devids"][index]=row[0];
		root["devtypeids"][index] = atoi(row[1]);
		if(row[2])
			root["companyids"][index] = row[2];
		if(row[3])
			root["planeids"][index] = atoi(row[3]);
		if(row[4])
			root["pilotids"][index]=atoi(row[4]);
		if(row[5] && string_to_json(row[5],info))
			root["infos"][index] = info;
		++index;
	}
	LOG(DEBUG,"select rows:%d",nums);
	backsuccess(sock,root,type,"select success");
	result_free(res);
	return true;
}

bool SqlWork::SBLXLB(Json::Value&root,int sock)
{
	int sqlLen = snprintf(sqlbuffer,max_sql_buffer_len,"select devtypeid,info from %s",table_name_devtype);
	if(sqlLen < 0){
		LOG(ERROR,"snprintf:%s",strerror(errno));
		return false;
	}
	std::string type="SBLXLB";
	root["devtypeids"].resize(0);
	root["infos"].resize(0);
	MYSQL_RES * res =  queryRes(sqlbuffer, sqlLen);
	if(res == NULL){
		LOG(ERROR,"queryres:%s",sqlbuffer);
		backfailure(sock,root,type,"not found records");
		return false;
	}
	int nums = 0;
	if((nums=getRowNums(res)) <= 0){
		LOG(ERROR,"select records:%d",nums);
	//	backfailure(sock,root,type,"found records zero");
	//	return false;
	}
	root["devtypeids"].resize(nums);
	root["infos"].resize(nums);
	MYSQL_ROW row;
	int index = 0;
	while((row=getRow(res)) && row[0]){ //row[0]对应的是主键
		Json::Value info;
		root["devtypeids"][index] = atoi(row[0]);
		root["infos"][index] = info;
		if(row[1] && string_to_json(row[1],info))
			root["infos"][index] = info;
		++index;
	}
	LOG(DEBUG,"select rows:%d",nums);
	backsuccess(sock,root,type,"select success");
	result_free(res);
	return true;
}

bool SqlWork::SJYLB(Json::Value&root,int sock)
{
	int sqlLen = snprintf(sqlbuffer,max_sql_buffer_len,"select ipaddr,devtypeid,info from %s",table_name_datasrc);
	if(sqlLen < 0){
		LOG(ERROR,"snprintf:%s",strerror(errno));
		return false;
	}
	std::string type="SJYLB";
	root["ipaddrs"].resize(0);
	root["devtypeids"].resize(0);
	root["infos"].resize(0);
	MYSQL_RES * res =  queryRes(sqlbuffer, sqlLen);
	if(res == NULL){
		LOG(ERROR,"queryRes:%s",sqlbuffer);
		backfailure(sock,root,type,"not found records");
		return false;
	}
	int nums = 0;
	if((nums=getRowNums(res)) <= 0){
		LOG(DEBUG,"select records:%d",nums);
	//	backfailure(sock,root,type,"found records zero");
	//	return false;
	}
	MYSQL_ROW row;
	int index = 0;
	root["ipaddrs"].resize(nums);
	root["devtypeids"].resize(nums);
	root["infos"].resize(nums);
	while((row=getRow(res)) && row[0]){ //row[0]对应的是主键
		Json::Value info;
		root["ipaddrs"][index] = row[0];
	//	root["devtypeids"][index] = -1;
		if(row[1])
			root["devtypeids"][index] = atoi(row[1]);
		root["infos"][index] = info;
		if(row[2] && string_to_json(row[2],info))
			root["infos"][index] = info;
		++index;
	}
	LOG(DEBUG,"select rows:%d",nums);
	backsuccess(sock,root,type,"select success");
	result_free(res);
	return true;
}

/*飞机列表*/
bool SqlWork::FJLB(Json::Value&root,int sock)
{
	std::string type = "FJLB";
	if(!root.isMember("companyid")){
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	std::string companyid = root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	int queryLen = snprintf(sqlbuffer,max_sql_buffer_len,"select planeid,info from %s where companyid=\'%s\'",
		table_name_plane,companyid.c_str());
	MYSQL_RES *res = queryRes(sqlbuffer,queryLen);
	if(res == NULL){
		backfailure(sock,root,type,"select err");
		return false;
	}
	MYSQL_ROW row = NULL;
	int index = 0;
	Json::Value planeids;
	Json::Value infos;
	planeids.resize(index);
	infos.resize(index);
	Json::Value val;
	while((row=mysql_fetch_row(res)) && row[0]){
		planeids.resize(index+1);
		infos.resize(index+1);
		planeids[index] = atoi(row[0]);
		if(row[1] && string_to_json(row[1],val)){
			infos[index] = val;
		}
		++index;
	}
	root["planeids"] = planeids;
	root["infos"] = infos;
	backsuccess(sock,root,type,"");
	result_free(res);
	return true;
}

/*飞行员列表*/
bool SqlWork::FXYLB(Json::Value&root,int sock){
	std::string type = "FXYLB";
	if(!root.isMember("companyid")){
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	std::string companyid = root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	int queryLen = snprintf(sqlbuffer,max_sql_buffer_len,"select pilotid,info from %s where companyid=\'%s\'",
		table_name_pilot,companyid.c_str());
	MYSQL_RES *res = queryRes(sqlbuffer,queryLen);
	if(res == NULL){
		backfailure(sock,root,type,"select err");
		return false;
	}
	MYSQL_ROW row = NULL;
	int index = 0;
	Json::Value pilotids;
	Json::Value infos;
	pilotids.resize(index);
	infos.resize(index);
	Json::Value val;
	while((row=mysql_fetch_row(res)) && row[0]){
		pilotids.resize(index+1);
		infos.resize(index+1);
		pilotids[index] = atoi(row[0]);
		if(row[1] && string_to_json(row[1],val)){
			infos[index] = val;
		}
		++index;
	}
	root["pilotids"] = pilotids;
	root["infos"] = infos;
	backsuccess(sock,root,type,"");
	result_free(res);
	return true;

}

#if 0
{ 
	head:”SW”
	type:”YHLB”
	content:{
		companyid:string
		userinfo:[json,json,…]
	}
}\r\n

注：json:{
	userid:string,
	permission:int
	planeids:jsonarray
	Info:jsonobject
}
#endif

/*用户列表*/
bool SqlWork::YHLB(Json::Value&root,int sock)
{
	Json::Value tmpValue;
	tmpValue.resize(0);
	root["userinfo"] = tmpValue;
	
	std::string type = "YHLB";
	if(!root.isMember("companyid")){
		backfailure(sock,root,type,"need companyid");
		return false;
	}
	if(!root["companyid"].isString()){
		backfailure(sock,root,type,"companyid is not string");
		return false;
	}
	std::string companyid = root["companyid"].asString();
	LOG(DEBUG,"companyid:%s",companyid.c_str());
	int queryLen = snprintf(sqlbuffer,max_sql_buffer_len,"select userid,permission,planeids,info from %s where companyid=\'%s\'",
		table_name_user,companyid.c_str());
	MYSQL_RES *res = queryRes(sqlbuffer,queryLen);
	if(res == NULL){
		LOG(ERROR,"select err:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		backfailure(sock,root,type,"select err");
		return false;
	}
	
	MYSQL_ROW row;
	int index = 0;
	while((row=mysql_fetch_row(res)) && row[0]){
		Json::Value planeids;
		Json::Value info;
		tmpValue.resize(index+1);
		tmpValue[index]["userid"]= row[0];
		tmpValue[index]["permission"]=0;
		tmpValue[index]["planeids"].resize(0);
		tmpValue[index]["info"]=info;
		if(row[1]){
			tmpValue[index]["permission"] = atoi(row[1]);
		}
		if(row[2] && string_to_json(row[2],planeids)){
			tmpValue[index]["planeids"] = planeids;
		}
		if(row[3] && string_to_json(row[3],info)){
			tmpValue[index]["info"] = info;
		}
		++index;
	}
	root["userinfo"] = tmpValue;
	backsuccess(sock,root,type,"");
	result_free(res);
	return true;

}

/*公司列表*/
bool SqlWork::GSLB(Json::Value&root,int sock)
{
	int sqlLen = snprintf(sqlbuffer,max_sql_buffer_len,"select companyid,signcompanyids,info from %s",table_name_company);
	if(sqlLen < 0){
		LOG(ERROR,"snprintf:%s",strerror(errno));
		return false;
	}
	
	std::string type="GSLB";
	root["companyinfo"].resize(0);

	MYSQL_RES * res =  queryRes(sqlbuffer, sqlLen);
	if(res == NULL){
		LOG(ERROR,"queryres err:%s",sqlbuffer);
		backfailure(sock,root,type,"not found records");
		return false;
	}
		
	MYSQL_ROW row;
	int index=0;
	while((row=getRow(res)) && row[0]){
		Json::Value info;
		Json::Value signcompanyids;
		signcompanyids.resize(0);
		
		root["companyinfo"].resize(index+1);
		root["companyinfo"][index]["companyid"] = row[0];
		root["companyinfo"][index]["signcompanyids"] = signcompanyids;
		root["companyinfo"][index]["info"] = info;
		if(row[1] && string_to_json(row[1],signcompanyids))
			root["companyinfo"][index]["signcompanyids"] = signcompanyids;
		if(row[2] && string_to_json(row[2],info))
			root["companyinfo"][index]["info"] = info;
		++index;
	}
	
	LOG(DEBUG,"select rows:%d",getRowNums(res));
	backsuccess(sock,root,type,"selected");
	result_free(res);
	return true;
}

bool SqlWork::RZFX(Json::Value&root,int sock)
{
	if(root["year"].isNull() || 
		root["month"].isNull()||
		root["day"].isNull()||
		root["querytype"].isNull()){
		LOG(ERROR,"err element of json:%s",root.toStyledString().c_str());
		return false;
	}

	if(!root["year"].isInt() || 
		!root["month"].isInt()||
		!root["day"].isInt()||
		!root["querytype"].isString()){
		LOG(ERROR,"err type of json:%s",root.toStyledString().c_str());
		return false;
	}
	
	int year = root["year"].asInt();
	int month = root["month"].asInt();
	int day = root["day"].asInt();
	std::string querytype = root["querytype"].asString();
	char daytable[100];
	snprintf(daytable,100,"data%d%02d%02d",year,month,day); 
	LOG(DEBUG,"querytype:%s",querytype.c_str());
	if(querytype == "plane"){
		class RZFX<PlaneRZ> rzfx;
		rzfx.Select(sqlfd,daytable,root,sock);
	}
	else if(querytype == "pilot"){
		class RZFX<PilotRZ> rzfx;
		rzfx.Select(sqlfd,daytable,root,sock);
	}
	else if(querytype == "dev"){
		class RZFX<DevRZ> rzfx;
		rzfx.Select(sqlfd,daytable,root,sock);
	}
}

MYSQL_RES *query_res(MYSQL &sqlfd,const char *sqlbuf,int sqllen)
{
	if(mysql_real_query(&sqlfd,sqlbuf,sqllen)){
		LOG(ERROR,"query err:%s",sqlbuf);
		return NULL;
	}

	MYSQL_RES *res = mysql_store_result(&sqlfd);
	if(res == NULL){
		LOG(DEBUG,"mysql_result err:%s,%s",sqlbuf,mysql_error(&sqlfd));
		return NULL;
	}
	return res;	
}

Json::Value PilotRZ::selectTimeInfo(int pilotid)
{
	Json::Value timeValue;
	timeValue.resize(0);
	int sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),"select min(packtime),max(packtime) from %s \
		where pilotid=%d and packtime>0",dayTableName.c_str(),pilotid);
		
	MYSQL_RES*res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res==NULL){
		LOG(ERROR,"query_res:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		return timeValue;
	}
	
	if(mysql_num_rows(res) == 0){
		LOG(ERROR,"mysql_num_rows==0");
		mysql_free_result(res);
		return timeValue;
	}

	int minTime = -1;
	int maxTime = -1;
	
	MYSQL_ROW row = mysql_fetch_row(res);
	if(row&&row[0])
		minTime = atoi(row[0]);
	if(row&&row[1])
		maxTime = atoi(row[1]);
	//maxtime不允许为0
	if(minTime<0 || maxTime<=0){
		LOG(ERROR,"mintime and maxtime error:%d,%d",minTime,maxTime);
		mysql_free_result(res);
		return timeValue;
	}
	mysql_free_result(res);//释放第一次查询的结果集

	//查询断开的时间
	sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),"select start,(select min(packtime) from %s where \
		pilotid=%d and packtime>start)as end from (select packtime as start from %s where packtime+1 not\
		in(select packtime from %s where pilotid=%d) and packtime<%d and pilotid=%d)as t",
		dayTableName.c_str(),pilotid,dayTableName.c_str(),dayTableName.c_str(),pilotid,maxTime,pilotid);
	res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res == NULL){
		LOG(ERROR,"query_res:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		return timeValue;
	}
	int start;
	int end;
	int index = 0;
	Json::Value timeRoot;
	timeRoot.resize(2);
	const int timeIndex0=0;
	const int timeIndex1=1;
	while((row = mysql_fetch_row(res)) && row[0] && row[1]){
		start = atoi(row[0]);
		end = atoi(row[1]);
		if(start < minTime)
			continue;
		if((end - start) > breakTimeValue){
			timeRoot[timeIndex0] = minTime;
			timeRoot[timeIndex1] = start;
			timeValue[index] = timeRoot;
			minTime = end;
			++index;
		}
#ifdef __DEBUG__
		LOG(DEBUG,"startbreak:%s",row[0]);
		LOG(DEBUG,"endbreak:%s",row[1]);
#endif
	}

	timeRoot[timeIndex0] = minTime;
	timeRoot[timeIndex1] = maxTime;
	timeValue[index] = timeRoot;
	mysql_free_result(res);
	return timeValue;
}


bool PilotRZ::Select(MYSQL &sqlfd,std::string tablename,Json::Value &root,int sock)
{
	this->sqlfd = sqlfd;
	this->dayTableName = tablename;
	this->root = root;
	this->sock = sock;
	LOG(DEBUG,"dayTableName:%s",dayTableName.c_str()); 

	Json::Value backroot;
	backroot["head"] = "SW";
	backroot["type"] = "RZFX";
	root["data"]["pilotids"].resize(0);
	root["data"]["timeinfos"].resize(0);
	backroot["content"] = root;

	int sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),"select distinct pilotid from %s",dayTableName.c_str());
	MYSQL_RES * res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res == NULL){
		LOG(DEBUG,"res==NULL:%s",sqlbuffer);
		sendJson(sock,backroot);
		return false;
	}
	
	MYSQL_ROW row;
	int index = 0;
	Json::Value pilotids;
	Json::Value timeinfo;
	int nums = mysql_num_rows(res);
	LOG(DEBUG,"nums:%d",nums);
	pilotids.resize(0);
	timeinfo.resize(0);
	while((row=mysql_fetch_row(res)) && row[0]){
		int pilotid = atoi(row[0]);
		LOG(DEBUG,"pilotid:%d",pilotid);
		if(pilotid == 0){ // 0表示无效值
			continue;
		}
		pilotids.resize(index+1);//更新组的大小
		timeinfo.resize(index+1);
		pilotids[index] = pilotid;
		timeinfo[index] = selectTimeInfo(pilotid);
		++index;
	}
	root["data"]["pilotids"] = pilotids;
	root["data"]["timeinfos"] = timeinfo;
	backroot["content"] = root;
	sendJson(sock,backroot);
	mysql_free_result(res);
	return true;
}

Json::Value PlaneRZ::selectTimeInfo(int planeid)
{
	Json::Value timeValue;
	timeValue.resize(0);
	int sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),"select min(packtime),max(packtime) from %s \
		where planeid=%d and packtime>0",dayTableName.c_str(),planeid);
		
	MYSQL_RES*res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res==NULL){
		LOG(ERROR,"query_res:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		return timeValue;
	}
	
	if(mysql_num_rows(res) == 0){
		LOG(ERROR,"mysql_num_rows==0");
		mysql_free_result(res);
		return timeValue;
	}

	int minTime = -1;
	int maxTime = -1;
	
	MYSQL_ROW row = mysql_fetch_row(res);
	if(row&&row[0])
		minTime = atoi(row[0]);
	if(row&&row[1])
		maxTime = atoi(row[1]);


	//maxtime不允许为0
	if(minTime<0 || maxTime<=0){
		LOG(ERROR,"mintime and maxtime error:%d,%d",minTime,maxTime);
		mysql_free_result(res);
		return timeValue;
	}
	mysql_free_result(res);//释放第一次查询的结果集
	//查询断开的时间
	//需要查询的结果是start、end。
#if 0
/*
*********************1
语句:select packtime as start from datatable where packtime+1 not in\
(select packtime from %s where planeid=%d) and packtime<%d and planeid=%d)as t",
语句作用：查询start时间。查询的结果作为t
注意条件:packtime<%d，指要小于最大时间数
注意条件:planeid=%d，指被查的飞机id
注意条件:packtime+1 not in (select packtime from %s where planeid=%d)表示packtime+1不在表中。
因为时间是连续的，例如表中时间为1、2、3、5、6、9,那么start结果为3、6（9是最大被条件过滤）;
最后生成一个结果集合t;

**********************2
语句：select start,(select min(packtime) from %s where \
		planeid=%d and packtime>start)as end from t;
语句作用：获得		start时间，end时间
start时间：从结果集t中获得
end时间：查询语句是：(select min(packtime) from %s where \
		planeid=%d and packtime>start)as end
	表示:从表中获取“最小”的packtime,条件是大于start（start已经在t结果集合中），例如表中packtime是：1、2、4、6、8、9、10
	那么，start是：2、4、6;end对应的是：4,6,8 。这里的8又作为start时间，10（最大时间）作为end时间，已在程序中处理。

*/
#endif

	sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),"select start,(select min(packtime) from %s where \
		planeid=%d and packtime>start)as end from (select packtime as start from %s where packtime+1 not\
		in(select packtime from %s where planeid=%d) and packtime<%d and planeid=%d)as t",
		dayTableName.c_str(),planeid,dayTableName.c_str(),dayTableName.c_str(),planeid,maxTime,planeid);
	res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res == NULL){
		LOG(ERROR,"query_res:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		return timeValue;
	}
	int start;
	int end;
	int index = 0;
	Json::Value timeRoot;
	timeRoot.resize(2);
	const int timeIndex0=0;
	const int timeIndex1=1;
	while((row = mysql_fetch_row(res)) && row[0] && row[1]){
		start = atoi(row[0]);
		if(start < minTime)
			continue;
		end = atoi(row[1]);
		if((end - start) > breakTimeValue){
			timeRoot[timeIndex0] = minTime;
			timeRoot[timeIndex1] = start;
			timeValue[index] = timeRoot;
			minTime = end;
			++index;
		}
#ifdef __DEBUG__
		LOG(DEBUG,"startbreak:%s",row[0]);
		LOG(DEBUG,"endbreak:%s",row[1]);
#endif

	}

	timeRoot[timeIndex0] = minTime;
	timeRoot[timeIndex1] = maxTime;
	timeValue[index] = timeRoot;
	mysql_free_result(res);
	return timeValue;
}


bool PlaneRZ::Select(MYSQL &sqlfd,std::string tablename,Json::Value &root,int sock)
{	
	this->sqlfd = sqlfd;
	this->dayTableName = tablename;
	this->root = root;
	this->sock = sock;
	LOG(DEBUG,"dayTableName:%s",dayTableName.c_str()); 
	
	Json::Value backroot;
	backroot["head"]="SW";
	backroot["type"]="RZFX";
	root["data"]["planeids"].resize(0);
	root["data"]["timeinfos"].resize(0);
	backroot["content"] = root;

	int sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),"select distinct planeid from %s",tablename.c_str());
	MYSQL_RES * res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res == NULL){//查询失败
		LOG(DEBUG,"res==NULL:%s",sqlbuffer);
		sendJson(sock,backroot);
		return false;
	}
	
	MYSQL_ROW row;
	int index = 0;
	Json::Value planeids;
	Json::Value timeinfo;
	int nums = mysql_num_rows(res);
	LOG(DEBUG,"nums:%d",nums);
	planeids.resize(0);
	timeinfo.resize(0);
	while((row=mysql_fetch_row(res)) && row[0]){
		int planeid = atoi(row[0]);
		LOG(DEBUG,"planeid:%d",planeid);
		if(planeid == 0){ // 0表示无效值
			continue;
		}
		planeids.resize(index+1);//更新组的大小
		timeinfo.resize(index+1);
		planeids[index] = planeid;
		timeinfo[index] = selectTimeInfo(planeid);
		++index;
	}
	root["data"]["planeids"] = planeids;
	root["data"]["timeinfos"] = timeinfo;
	backroot["content"] = root;
	sendJson(sock,backroot);
	mysql_free_result(res);
	return true;
}


bool DevRZ::Select(MYSQL &sqlfd,std::string tablename,Json::Value &root,int sock)
{
	this->sqlfd = sqlfd;
	this->dayTableName = tablename;
	this->root = root;
	this->sock = sock;
	LOG(DEBUG,"tablename:%s",dayTableName.c_str());

	Json::Value backroot;
	backroot["head"]="SW";
	backroot["type"]="RZFX";
	root["data"]["devids"].resize(0);
	root["data"]["devtypeids"].resize(0);
	root["data"]["timeinfos"].resize(0);
	backroot["content"] = root;
	int sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),
		"select distinct ipaddr,(select devtypeid from %s where ipaddr=a.ipaddr)as devtypeid,devid from %s a",
		table_name_datasrc,dayTableName.c_str());
	MYSQL_RES * res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res == NULL){
		LOG(ERROR, "res==NULL:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		sendJson(sock,backroot);
		return false;
	}

	MYSQL_ROW row;
	int index = 0;
	Json::Value devids;
	Json::Value devtypeids;
	Json::Value timeinfo;
	int nums = mysql_num_rows(res);
	LOG(DEBUG,"nums:%d",nums);
	devids.resize(0);
	devtypeids.resize(0);
	timeinfo.resize(0);
	while(row=mysql_fetch_row(res)){
		if(row[0] == NULL){
			LOG(DEBUG,"row[0]==NULL");
			continue;
		}

		if(row[1] == NULL){
			LOG(DEBUG,"ippadr:%s:row[1]==NULL",row[0]);
			continue;
		}
		
		if(row[2] == NULL){
			LOG(DEBUG,"ippadr:%s:row[2]==NULL",row[0]);
			continue;
		}
		
		std::string ipaddr = row[0];
		int devtypeid = atoi(row[1]);
		std::string devid = row[2];
#ifdef __DEBUG__
		LOG(DEBUG,"ipaddr:%s",ipaddr.c_str());
		LOG(DEBUG,"devtypeid:%d",devtypeid);
		LOG(DEBUG,"devid:%s",devid.c_str());
#endif
		devids.resize(index+1);
		devtypeids.resize(index+1);
		timeinfo.resize(index+1);
		
		devids[index] = row[2];
		devtypeids[index]=atoi(row[1]);
		timeinfo[index] = selectTimeInfo(ipaddr,devid);
		++index;
	}

	root["data"]["devids"] = devids;
	root["data"]["devtypeids"] = devtypeids;
	root["data"]["timeinfos"] = timeinfo;
	backroot["content"] = root;
	sendJson(sock,backroot);
	mysql_free_result(res);
	return true;
}

Json::Value DevRZ:: selectTimeInfo(const std::string & ipaddr,const std::string &devid)
{
	Json::Value timeValue;
	timeValue.resize(0);
	int sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),"select min(packtime),max(packtime) from %s \
		where ipaddr=\'%s\' and devid=\'%s\' and packtime>0",dayTableName.c_str(),ipaddr.c_str(),devid.c_str());
	MYSQL_RES*res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res==NULL){
		LOG(ERROR,"query_res:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		return timeValue;
	}

	if(mysql_num_rows(res) == 0){
		LOG(ERROR,"mysql_num_rows==0");
		mysql_free_result(res);
		return timeValue;
	}

	int minTime = -1;
	int maxTime = -1;
	
	MYSQL_ROW row = mysql_fetch_row(res);
	if(row&&row[0])
		minTime = atoi(row[0]);
	if(row&&row[1])
		maxTime = atoi(row[1]);
#ifdef __DEBUG__
	LOG(DEBUG,"min time:%d",minTime);
	LOG(DEBUG,"max time:%d",maxTime);
#endif

	//maxtime不允许为0
	if(minTime<0 || maxTime<=0){
		LOG(ERROR,"mintime and maxtime error:%d,%d",minTime,maxTime);
		mysql_free_result(res);
		return timeValue;
	}
	mysql_free_result(res);//释放第一次查询的结果集

	//查询断开的时间
	sqllen = snprintf(sqlbuffer,sizeof(sqlbuffer),"select start,(select min(packtime) from %s where \
		devid=\'%s\' and ipaddr=\'%s\' and packtime>start)as end from (select packtime as start from %s where packtime+1 not\
		in(select packtime from %s where devid=\'%s\' and ipaddr=\'%s\') and packtime<%d and devid=\'%s\' and ipaddr=\'%s\')as t",
		dayTableName.c_str(),devid.c_str(),ipaddr.c_str(),dayTableName.c_str(),dayTableName.c_str(),devid.c_str(),ipaddr.c_str(),\
		maxTime,devid.c_str(),ipaddr.c_str());
	res = query_res(sqlfd,sqlbuffer,sqllen);
	if(res == NULL){
		LOG(ERROR,"query_res:%s,%s",sqlbuffer,mysql_error(&sqlfd));
		return timeValue;
	}
#ifdef __DEBUG__
	LOG(DEBUG,"query suc:%s",sqlbuffer);
#endif
	int start =0;
	int end = 0;
	int index = 0;
	Json::Value timeRoot;
	timeRoot.resize(2);
	const int timeIndex0=0;
	const int timeIndex1=1;
	while((row = mysql_fetch_row(res)) && row[0] && row[1]){
		start = atoi(row[0]);
		end = atoi(row[1]);
		if(start < minTime)
			continue;
		if((end - start) > breakTimeValue){
			timeRoot[timeIndex0] = minTime;
			timeRoot[timeIndex1] = start;
			timeValue[index] = timeRoot;
			minTime = end;
			++index;
		}
#ifdef __DEBUG__
		LOG(DEBUG,"beginbreak:%s",row[0]);
		LOG(DEBUG,"finialbreak:%s",row[1]);
#endif
	}
	
#ifdef __DEBUG__
	LOG(DEBUG,"mintime:%d",minTime);
	LOG(DEBUG,"maxtime:%d",maxTime);
#endif
	timeRoot[timeIndex0] = minTime;
	timeRoot[timeIndex1] = maxTime;
	timeValue[index] = timeRoot;
	mysql_free_result(res);
	return timeValue;
}
