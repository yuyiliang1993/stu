#include <decoderHead.h>
#include <Prot_TH.h>
#include <PackBase.h>
#include <time.h>
#include <float.h>
#include<iostream>
#include "decodeData.h"
#include "myzlog.h"
#include <string.h>
#include <stdio.h>
extern const char *table_name_datasrc;
extern const char *table_name_dev;

SSFX* SSFX::getInstance(void){
	if(instance == NULL){
		instance = new SSFX;
	}
	return instance;
}

void SSFX::destoryInst(void){
	if(instance){
		delete instance;
		instance = NULL;
	}
}

bool SSFX::saveAndTransmit(MYSQL&sqlfd,Json::Value &root,const ManageClients&manageCli)
{
//	LOG(DEBUG,"start savetotable");
	rtData.saveToTable(sqlfd, root);
//	LOG(DEBUG,"end savetotable");
	transToWebClient(manageCli);
}

#if 0
Protocol:string
Ipaddr:string
Devid:int
Planeid:int
Pilotid:int
recvtime:int 
data:json
#endif

bool SSFX::transToWebClient(const ManageClients&manageCli)
{
	Json::Value root;
	root["head"]="SW";
	root["type"]="SS";
	root["content"]["protocol"] = rtData.protocol;
	root["content"]["ipaddr"] = rtData.ipaddr;
	root["content"]["devid"] = rtData.devid;
	root["content"]["planeid"] = rtData.planeid;
	root["content"]["pilotid"] = rtData.pilotid;
	root["content"]["recvtime"] = rtData.recvtime;
	root["content"]["data"] = rtData.decodedata;
	manageCli.sendToClient("555555",root);
	manageCli.sendToClient("123456",root);
	return true;
}


SSFX * SSFX::instance = NULL;

void show_hex(std::vector<BYTE> &bytesbuffer)
{
	fprintf(stdout,"hex:");
	for(int i=0;i<bytesbuffer.size();++i){
		fprintf(stdout,"%02x ",bytesbuffer[i]);
	}
	fprintf(stdout,"\n");
}

bool showHex(std::vector<BYTE> &bytesbuffer){
	fprintf(stdout,"hex:");
	for(int i=0;i<bytesbuffer.size();++i){
		fprintf(stdout,"%02x ",bytesbuffer[i]);
	}
	fprintf(stdout,"\n");
}


bool StrToByte::stringToBytes(const std::string &str)
{
	int slen = str.size();
	const char * pstr = str.c_str();
	if(slen == 0 || slen % 2){
		LOG(ERROR,"len of str error");
		return false;
	}
	char temp[2];
	this->bytesBuffer.clear();
	while(slen > 0){
		memcpy(temp,pstr,2);
		BYTE a = ::strtol(temp,NULL,16);
		this->bytesBuffer.push_back(a);
		pstr+=2;
		slen-=2;
	}
	return true;
}

bool RealTimeData::parseJsonData(MYSQL &sqlfd,const Json::Value &root)
{
	init();
	if(root["data"].isNull() || root["ipaddr"].isNull()){
		return false;
	}
	if(!root["ipaddr"].isString() || !root["data"].isObject()){
		return false;
	}
	//(1)解码获取packtime和devid
	if(decodeStringData(root["data"]) == false){//packtime,devid
		return false;
	}

	//(2)获取接收时间
	time_t utc = time(NULL);
	tm = localtime(&utc);
	int localdaytime = (int)(tm->tm_hour*3600+tm->tm_min*60+tm->tm_sec);//recvtime
	recvtime = utc;
	//(3)获取idaddr
	ipaddr = root["ipaddr"].asString();//ipaddr
	
	//(4)data转成字符串
	data = w.write(root["data"]);
	if(data[data.size()-1] == '\n')
		data[data.size()-1] = 0;
	//(5)查表获取planeid和devid
	if(select_dev_table(sqlfd,ipaddr) == false){
		return false;
	}

	if(localdaytime - packtime > 5){
		LOG(WARN,"localdaytime - packtime >5:%d,%d",localdaytime,packtime);
		packtime = localdaytime;
	}
#ifdef __DEBUG__	
	LOG(DEBUG,"devid:%s",devid.c_str());
	LOG(DEBUG,"devtypeid:%d",devtypeid);
	LOG(DEBUG,"planeid:%d",planeid);
	LOG(DEBUG,"pilotid:%d",pilotid);

	LOG(DEBUG,"data:%s",data.c_str());
	LOG(DEBUG,"ipaddr:%s",ipaddr.c_str());
	LOG(DEBUG,"recvtime:%d",recvtime);
	LOG(DEBUG,"packtime:%d",packtime);
#endif
	return true;
}

bool RealTimeData::decodeStringData(const Json::Value &data)
{
	std::string datastring;
	switch(dataType(data)){
		case XC:
			datastring = data["XC"].asString();
			break;
		case SC:
			datastring = data["SC"].asString();
			break;
		case RW:
			break;
		default:
			return false;
	}
	
	if(datastring.size() == 0){
		LOG(ERROR,"size of data string is error");
		return false;
	}

	StrToByte _tohex;
	if(_tohex.stringToBytes(datastring) == false){
		LOG(ERROR,"_tohex.stringToHex() failed");
		return false;
	}
#ifdef __DEBUG__
	LOG(DEBUG,"datastring:%s",datastring.c_str());
	show_hex(_tohex.bytesBuffer);
#endif
	if(decodeByteMsg(_tohex.bytesBuffer) == false){
		LOG(ERROR,"decodeByteMsg() failed");
		return false;
	}
	return true;
}

bool RealTimeData::decodeByteMsg(const std::vector<BYTE> &buf){
	if(buf.size() <= 0)
		return false;
	CDecoder _decoder;
	PTR<CProt_TH> _Prot_TH( dynamic_cast<CProt_TH*>( CProt_TH::Prot().operator->() ));			
    PTR<CProtocolShell::unShelledPack> _decodeRes = _decoder.DecodeMsg(buf, _Prot_TH );
	std::vector<PTR<const CPackBase>> _decodedPacks =  _decoder.getLastPack();
	std::vector<PTR<const CPackBase>>::iterator _pack = _decodedPacks.begin();
	if(_decodeRes->decodedSize > 0 && _decodedPacks.size() > 0){
		devid =  std::to_string(_decodeRes->senderID);
		packtime = _decodeRes->time;
		decodedata = (*_pack)->toString();
		if(( *_pack )->isSameType( _Prot_TH->m_zywd )){
			LOG(DEBUG,"packstring:%s",decodedata.c_str());
		}
		protocol = _decodeRes->protocol;
		return true;	
   	}
	return false;
}


RealTimeData::RET_JOSN_T RealTimeData::dataType(const Json::Value &data){
	return data.isMember("SC")?SC:data.isMember("XC")?XC:data.isMember("RW")?RW:ERR;
}

bool RealTimeData::select_dev_table(MYSQL &sqlfd,const std::string &ipaddr)
{
	int queryLen =snprintf(querybuffer,maxBufferLen,"select devtypeid,(select planeid from devInfo where devid=\'%s\' and devtypeid=a.devtypeid)as planeid ,\
		(select pilotid from devInfo where devid=\'%s\' and devtypeid=a.devtypeid)as pilotid from datasrcInfo a where ipaddr=\'%s\'",
		devid.c_str(),devid.c_str(),ipaddr.c_str());
	if(mysql_real_query(&sqlfd,querybuffer,queryLen)){
		LOG(ERROR,"query err:%s,%s",querybuffer,mysql_error(&sqlfd));
		return false;
	}
	//获取结果集合
	MYSQL_RES *res = mysql_store_result(&sqlfd);
	if(res == NULL){
		LOG(ERROR,"res==NULL:%s,%s",querybuffer,mysql_error(&sqlfd));
		return false;
	}
	//读取一行
	MYSQL_ROW row = mysql_fetch_row(res);
	if(row){
			if(row[0])
				devtypeid = atoi(row[0]);
			if(row[1])
				planeid = atoi(row[1]);
			if(row[2])
				pilotid = atoi(row[2]);
	}
	//释放结果集
	mysql_free_result(res);	
	return true;
}

bool RealTimeData::saveToTable(MYSQL&sqlfd,Json::Value &root)
{
	//(1)解析数据
	if(parseJsonData(sqlfd,root) == false){
		return false;
	}
	//(2)动态创建数据表
	int queryLen = snprintf(querybuffer,maxBufferLen,"create table if not exists data%d%02d%02d(seq int auto_increment,\
	ipaddr varchar(256),packtime int,recvtime int,planeid int,pilotid int,devid varchar(256),data text,devtypeid int,\
	primary key(seq))ENGINE = InnoDB DEFAULT CHARSET = utf8",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday);
	if(mysql_real_query(&sqlfd,querybuffer,queryLen)){
		LOG(ERROR,"query err:%s",querybuffer);
		return false;
	}
	//(3)执行插入操作
	queryLen = snprintf(querybuffer,maxBufferLen,"insert into data%d%02d%02d values(NULL,\'%s\',%d,%d,%d,%d,\'%s\',\'%s\',%d)",
	tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,ipaddr.c_str(),packtime,recvtime,planeid,pilotid,devid.c_str(),data.c_str(),devtypeid);
	if(mysql_real_query(&sqlfd,querybuffer,queryLen)){
		LOG(DEBUG,"inert err:%s",querybuffer);
		return false;
	}
#ifdef __DEBUG__
	LOG(DEBUG,"inert:%s",querybuffer);
#endif
	return true;
}
