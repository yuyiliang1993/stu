#include "lshf.h"
#include "myzlog.h"
#include "apue.h"
#include <string>
#include <iostream>
#include "mutexLock.h"
#include "decodeData.h"
extern int senddata(int fd, const char * sendbuf, int len);
extern bool string_to_json(const char*str,Json::Value &val);

static int enPacksToSend(int sock,MYSQL_ROW row,int socketid)
{
	Json::Value data;
	Json::Value root;
	root["head"]="SW";
	root["type"]="LS";
	Json::Value content;
	content["ipaddr"] = "";
	content["planeid"] = 0;
	content["pilotid"] = 0;
	content["recvtime"] = 0;
	if(row && row[0] && row[2] && row[5] && row[6] && string_to_json(row[6],data)){
		static RealTimeData rtdata;
		rtdata.init();
		if(rtdata.decodeStringData(data)){
			content["protocol"]=rtdata.protocol;
			content["ipaddr"] = row[0];
			if(row[3])
				content["planeid"]=atoi(row[3]);
			if(row[4])
				content["pilotid"] = atoi(row[4]);
			content["devid"] = row[5];
			content["recvtime"] = atoi(row[2]);
			content["data"]=rtdata.decodedata;
			content["socketid"]=socketid;
			root["content"]=content;
			return sendJson(sock,root);
		}
	}
	return 0;
}

/*发送线程*/
static void *pthreadLoopSend(void *arg)
{
	int value = 0;
	MYSQL_ROW row;
	ReqArrData * p = & LSHF::requestArraydata;
	LshfInfo * arrays = p->LshfInfoRequestArrays;
	int maxArrayNums = p->maxArrayNums;
	int sendBytesSum = 0;
	int loopCountNums = 0;
	int sendbytes = 0;
	for(;;)
	{
		loopCountNums = 0;
		for(int i =0;i < maxArrayNums;++i){
			lock_mutex(&mutex);
			if(arrays[i].socketid >= 0 && arrays[i].res){
				if((row = mysql_fetch_row(arrays[i].res)) == NULL){
					LOG(DEBUG,"send complete:%ld rows",arrays[i].sumSendBytes);
					mysql_free_result(arrays[i].res);//释放结果集
					arrays[i].init();
				}else if((sendbytes = enPacksToSend(arrays[i].requestSock,row,arrays[i].socketid)) <= 0){
					LOG(DEBUG,"send socketid[%d],err:%s",arrays[i].socketid,strerror(errno));
					mysql_free_result(arrays[i].res);//释放结果集
					arrays[i].init();
				}
				else{
					++arrays[i].sumSendBytes;
				//	arrays[i].sumSendBytes += sendbytes;
					sendBytesSum += sendbytes;
				}
				++loopCountNums;
			}
			unlock_mutex(&mutex);
			if(sendBytesSum >= 512*1024){
				sendBytesSum = 0;
				sleep(1);
			}
		}
		if(0 == loopCountNums){ //无请求，线程休1s。为了cpu使用率
			sleep(1);
		}
	}
	LOG(DEBUG,"pthread exit");
	pthread_exit(0);
}
	
ReqArrData LSHF::requestArraydata;//请求组数据

std::mutex LSHF::m_mutex;

LSHF * LSHF::lshf = NULL;

pthread_t LSHF:: pid = 0;

LSHF *LSHF::getInstance(void){
	std::lock_guard<std::mutex> lock(m_mutex);
	if(lshf == NULL){
		lshf = new LSHF;
	}
	return lshf;
}

extern const char *table_name_company;
/*约定:线程中取的格式与搜索语句 的格式要一致*/
bool LSHF::start(MYSQL &sqlfd,Json::Value root,int sock)
{
	if(	!root.isMember("year") && \
		!root.isMember("month") &&\
		!root.isMember("day") &&\
		!root.isMember("sec") &&
		!root.isMember("socketid")){
		LOG(ERROR,"json err:%s",root.toStyledString().c_str());
		return false;
	}
	if(	!root["year"].isInt() ||
		!root["month"].isInt()||
		!root["day"].isInt()||
		!root["sec"].isInt() ||
		!root["socketid"].isInt()){
		LOG(ERROR,"json err:%s",root.toStyledString().c_str());
		return false;
	}

	int year = root["year"].asInt();
	int month = root["month"].asInt();
	int day = root["day"].asInt();
	int sec = root["sec"].asInt();
	int socketid = root["socketid"].asInt();

	LOG(DEBUG,"LSHF:year-%d,month-%d,day-%d,sec-%d,socketid-%d",year,month,day,sec,socketid);

	char dataFileName[50];
	memset(dataFileName,0x00,sizeof(dataFileName));
	snprintf(dataFileName,sizeof(dataFileName),"data%d%02d%02d",year,month,day);
	LOG(DEBUG,"LSHF:datafilename:%s",dataFileName);
	
	const int maxbufferlen = 8*1024;
	char * querybuffer = new char[maxbufferlen];
	if(!querybuffer){
		LOG(ERROR, "new querybuffer failed");
		return false;
	}
	MYSQL_RES *res = NULL;//在此函数中不要释放，线程中需要使用
	if(root.isMember("planeids")){
		if(!root["planeids"].isArray()){
			LOG(DEBUG,"planeids is not array");
			return false;
		}
		Json::Value planeids = root["planeids"];
		int querylen = snprintf(querybuffer,maxbufferlen,"select ipaddr,packtime,recvtime,planeid,pilotid,devid,data from %s where(",dataFileName);
		for(int i=0;i<planeids.size();++i){
			querylen+=snprintf(querybuffer+querylen,maxbufferlen-querylen,"planeid=%d OR ",planeids[i].asInt());
		}
		querylen += snprintf(querybuffer+querylen-4,maxbufferlen-querylen+4,")AND packtime>=%d",sec);
		res = queryResult(sqlfd,querybuffer,strlen(querybuffer));
		goto ADD;
	}
	
	if(root.isMember("pilotids")){
		if(!root["pilotids"].isArray()){
			LOG(DEBUG,"pilotids is not array");
			return false;
		}
		Json::Value pilotids = root["pilotids"];
		int querylen = snprintf(querybuffer,maxbufferlen,"select ipaddr,packtime,recvtime,planeid,pilotid,devid,data from %s where(",dataFileName);
		for(int i=0;i<pilotids.size();++i){
			querylen+=snprintf(querybuffer+querylen,maxbufferlen-querylen,"pilotid=%d OR ",pilotids[i].asInt());
		}
		querylen += snprintf(querybuffer+querylen-4,maxbufferlen-querylen+4,")AND packtime>=%d",sec);
		res = queryResult(sqlfd,querybuffer,strlen(querybuffer));
		goto ADD;
	}
	if(root.isMember("devids") && root.isMember("devtypeids")){
		if(!root["devids"].isArray() || !root["devtypeids"].isArray()){
			LOG(ERROR,"devids or devtypeids is not array");
			return false;
		}
		Json::Value devids = root["devids"];
		Json::Value devtypeids = root["devtypeids"];
		if(devids.size() != devtypeids.size()){
			LOG(ERROR,"devids.size() != devtypeids.size()");
			return false;
		}
		//获取所有的ipaddr*		
		int querylen = snprintf(querybuffer,maxbufferlen,\
		"select ipaddr,packtime,recvtime,planeid,pilotid,devid,data from %s where(",dataFileName);
		for(int i=0;i<devids.size();++i){
			querylen+=snprintf(querybuffer+querylen,maxbufferlen-querylen,\
				"(devid=\'%s\' and devtypeid=%d) OR ",devids[i].asString().c_str(),devtypeids[i].asInt());
		}
		querylen += snprintf(querybuffer+querylen-4,maxbufferlen-querylen+4,")AND packtime>=%d",sec);
		LOG(DEBUG,"LSHF:%s",querybuffer);
		res = queryResult(sqlfd,querybuffer,strlen(querybuffer));
		goto ADD;
	}	
ADD:	
	if(res == NULL){
		LOG(ERROR,"queryResult:%s,%s",mysql_error(&sqlfd),querybuffer);
		delete[] querybuffer;
		return false;
	}
	LOG(DEBUG,"sum rows:%d",getRowNums(res));

	insertArrays(res,socketid, sock);	

	if(isPthreadAlive() == false){//判断线程是否存在。
		if(pthread_create(&pid,NULL,pthreadLoopSend,NULL) < 0){
			LOG(ERROR,"pthread_create:%s",strerror(errno));
		}else {
			LOG(DEBUG,"pthread_create success");
		}
	}
	delete[] querybuffer;
}


bool LSHF::isPthreadAlive(void)
{
	if(pid == 0)
		return false;
	int ret = pthread_kill(pid,0);
	if(ret == ESRCH || ret == EINVAL)
		return false;
	else 
		return true;
}

void LSHF::destoryInst(void)
{
	if(lshf == NULL)
		return ;
	delete lshf;
	lshf = NULL;
}

bool LSHF::stop(Json::Value root)
{
	if(root["socketid"].isNull()){
		LOG(ERROR,"root[socketid] is null");
		return false;
	}
	else if(!root["socketid"].isInt()){
		LOG(ERROR,"root[socketid] is not int");
		return false;
	}
	int socketid = root["socketid"].asInt();
	int i = 0;
	lock_mutex(&mutex);//加锁
	for(i =0;i<requestArraydata.maxArrayNums;++i){
		if(requestArraydata.LshfInfoRequestArrays[i].socketid == socketid){
			freeResult(requestArraydata.LshfInfoRequestArrays[i].res);
			requestArraydata.LshfInfoRequestArrays[i].init();
			LOG(DEBUG,"stop lshf success:%d",socketid);
			break;
		}
	}
	unlock_mutex(&mutex);//解锁
	if(i == requestArraydata.maxArrayNums){
		LOG(DEBUG,"stop lshf failed:not found the socketid:%d",socketid);
		return false;
	}

	return true;
}

void LSHF::insertArrays(MYSQL_RES *res,int socketid,int requestSocket)
{
	lock_mutex(&mutex);//加锁
	for(int i =0;i<requestArraydata.maxArrayNums;++i){
		if(requestArraydata.LshfInfoRequestArrays[i].socketid == socketid || \
			requestArraydata.LshfInfoRequestArrays[i].socketid < 0){
			freeResult(requestArraydata.LshfInfoRequestArrays[i].res);
			requestArraydata.LshfInfoRequestArrays[i].res  = res;
			requestArraydata.LshfInfoRequestArrays[i].socketid = socketid;
			requestArraydata.LshfInfoRequestArrays[i].requestSock = requestSocket;
			break;
		}
	}
	unlock_mutex(&mutex);//解锁
}
void LshfInfo::init(void){
	res = NULL;
	socketid = -1;
	sumSendBytes = 0UL;
	requestSock = -1;
}

