#include "serverwork.h"
using namespace std;
#include "myzlog.h"
#include <string.h>
const char *IPADDR_CONN_TRANSSERV="10.0.0.99";
const int PORT_CONN_TRANSSERV = 8604;
int connSocket1 = -1;

/*static bool string_to_json(const char*str,Json::Value &val)
{
	 Json::Reader reader;
	 if(reader.parse(str,val)){ 
		 return true; 
	 }
	 return false;
}
*/
/*static int sendJson(int fd,Json::Value root)
{
	Json::FastWriter writer;
	std::string str =writer.write(root);
	str+="\r\n";
	return senddata(fd,str.c_str(),str.length());
}*/


ServerWork::ServerWork(int clientnums)
{
	mClient = new ManageClients(clientnums);	
	epfd = new CEpoll;
	sSocket = new CServSocket;
}

ServerWork::~ServerWork()
{
	close();
}

void ServerWork ::delfd(int fd,short event)
{
	if(fd>=0){
		epfd->event_del(fd,event);
		::close(fd);
	}
}

void ServerWork ::loopStart(void)
{
	if(sSocket->openListener("",7603) == false){
		LOG(ERROR,"openlistener failed:%s",strerror(errno));
		exit(0);
	}
	else 
		epfd->event_add(sSocket->getSockfd(),EV_READ);

	LOG(DEBUG,"listening...");
	while(1){
		disposeEvents(epfd->event_wait(1000));
	}
}

void ServerWork ::acceptClient(void)
{
	int connfd;
	sockaddr_in_t cliaddr;
	memset(&cliaddr,0,sizeof(cliaddr));
	if(sSocket->acceptSocket(connfd, cliaddr)){
		Client cli(1024*1024*10);//分配10M内存
		cli.fd = connfd;
		cli.addr = cliaddr;
		cli.idnum="";
		mClient->AddClient(cli);
		epfd->event_add(connfd,EV_READ);
		LOG(DEBUG,"client linked:%d,%s,%d",connfd,inet_ntoa(cliaddr.sin_addr),htons(cliaddr.sin_port));
	}
}

bool ServerWork::connect_socket1(std::string ip,int port)
{
	connectSocket1.loginToServer();
	if(connectSocket1.isBreaking() == false)
		return true;
	if(connectSocket1.connect(ip.c_str(),port)){
		LOG(DEBUG,"connectsocket1 success:%s,%d",ip.c_str(),port);
		if(epfd->event_add(connectSocket1.getSockfd(),EV_READ)){
			LOG(DEBUG,"epoll.addevent(connectSocket1) success");
			connectSocket1.setLoginValue(true);
			connSocket1  = connectSocket1.getSockfd();
			return true;
		}
		LOG(DEBUG,"epoll.addevent(connectSocket1) err:%s",strerror(errno));
		connectSocket1.close();
	}
	LOG(DEBUG,"connectsocket1 failed:%s,%d",ip.c_str(),port);
	return false;
}

void ServerWork ::disposeEvents(int eventnums)
{
	if(eventnums < 0){
		LOG(ERROR,"wait err:%s",strerror(errno));
		exit(0);
	}
	int index = -1,fd = -1; 
	epoll_event_t *pevent = epfd->getActiveEvents();
	for(int i =0;i<eventnums;++i)
	{
		fd = pevent[i].data.fd;
		if(pevent[i].events & EV_READ){
			if(isListenSocket(fd)){
				acceptClient();
				continue;
			}
			if((index = mClient->GetClientIndex(fd)) >= 0){
				if(mClient->ReadClient(index) == false){
					epfd->event_del(fd,EV_READ);
					mClient->DelClient(index);
				}
				continue;
			}
			if(isConnectSocket1(fd)){
				if(connectSocket1.connect_event_read(*mClient) == false){
					epfd->event_del(fd,EV_READ);
				}
				continue;
			}
			else
				delfd(fd,EV_READ);
		}
	}
}

bool ServerWork::isListenSocket(int fd){
	return (fd == sSocket->getSockfd());
}

bool ServerWork::isConnectSocket1(int fd){
	return (fd==connectSocket1.getSockfd());
}

void ServerWork:: close(void){
	if(mClient){
		delete mClient;
		mClient = 0;
	}
	if(epfd){
		delete epfd;
		epfd = 0;
	}
	if(sSocket){
		delete sSocket;
		sSocket = 0;
	}
}

#include "decodeData.h"

ConnectSocket::ConnectSocket(int maxBufferLen):cbuffer(maxBufferLen),localId(777777){
	std::cout<<"cbuffer maxBufferLen:"<<maxBufferLen<<std::endl;
	mclient = NULL;
	loginValue = false;
}

bool stringToJson(const char*str,Json::Value &val)
{
	 Json::Reader reader;
	 if(reader.parse(str,val)){ 
		 return true; 
	 }
	 return false;
}

bool ConnectSocket::initConnectMYSQL(void){
	if(connSql.IsConnectionBreak()){
		if(connSql.Connect("127.0.0.1","root","shunying", "webdatas")){
			LOG(DEBUG,"connectSocket:connect mysql success");
			connSql.SetOptReconnect();
		}
	}
	return true;
}


bool ConnectSocket::dataJsonDispose(Json ::Value &root){

#ifdef __DEBUG__
	Json::FastWriter writer;
	std::string data = writer.write(root);
	if(data[data.size()-1] == 0x0a)
		data[data.size()-1] = 0;
	LOG(DEBUG,"recv connsocket1:%s",data.c_str());
#endif
	if(!root.isMember("type") || !root["type"].isString()){
		return false;
	}
	if(!root.isMember("content") || !root["content"].isObject()){
		return false;
	}
	
	Json::Value content = root["content"];
	std::string type = root["type"].asString();
	initConnectMYSQL();	
	if(type == "SSFX"){
		class SSFX*ssfx = SSFX::getInstance();
		return ssfx->saveAndTransmit(connSql.GetFd(),content,*mclient);
	}
	return true;
}

bool ConnectSocket::isIdStatus(const char *dat,int len)
{
	if(dat == NULL||len == 0)
		return false;
	const char *p = dat;
	int i = 0;
	for(i = 0;i < len-5;i++)
	{
		if(	p[i+0] == '$' && \
			p[i+1] == 'I' && \
			p[i+2] == 'D' && \
			p[i+3] == 'S' && len - i >= (unsigned char)p[i+4])
		{
			
			if(p[i+5] == 0x00){
				setLoginValue(true);
			}
			else if(p[i+5] == 0x01){
				LOG(DEBUG,"login server succcess");
				setLoginValue(false);
			}
			return true;
		}
	}
	return false;
}

int getCheckSum(unsigned char *dat,int len)
{
	unsigned char *p = dat;
	int i = 0;
	int sum =0;
	for(i = 0;i < len;i++){
		sum += p[i];
	}
	return (0xff -sum +1);
}

void loginIntId(int fd,int id)
{
	unsigned char buf[20]={'$','I','D',9};

	buf[4]|= id>>16;buf[5]|= id>>8;buf[6]|= id>>0;

	buf[7]= '*';

	buf[8]= ::getCheckSum(buf,7);

	::write(fd,buf,9);
}

void ConnectSocket::setLoginValue(bool value){
	loginValue = value;
}

void ConnectSocket::loginToServer(){
	if(loginValue){
		LOG(DEBUG,"login:%d",localId);
		loginIntId(getSockfd(),localId);
	}	
}

bool ConnectSocket::extractJsonData(const char *data){
	Json::Value root;
	if(stringToJson(data,root)){
		dataJsonDispose(root);
		return true;
	}
	return false;
}

bool ConnectSocket::buffer_handle(){
	char *const buffer = cbuffer.buffer;
	int &offsetlen = cbuffer.offetLens;
	buffer[offsetlen] = 0;
	const char *p = buffer;
	int packnums = 0;
	int dlen = 0;
	while(p)
	{
		if(*p != '{'){
			++p;
			if(p == buffer+offsetlen){
				offsetlen = 0;
				break;
			}
			continue;
		}
		const char *pstr = strstr(p,"\r\n");
		if(pstr == NULL){
			if(packnums > 0){
				dlen = strlen(p);
				memcpy(buffer,p,dlen);
				offsetlen = dlen;
				packnums = 0;
			}
			break;
		}else{
			++packnums;
			extractJsonData(p);
		}
		p = pstr;
	}
	if(packnums > 0){
		offsetlen = 0;
	}
}

bool ConnectSocket::connect_event_read(const ManageClients&mclient){
	this->mclient = &mclient;//赋值一次
	char * const buffer = this->cbuffer.buffer;
	int &len = this->cbuffer.offetLens;
	int maxlen = this->cbuffer.totalLens;
	if(len+256 > maxlen) // 存在丢数据
		len = 0;
	int rbytes = ::recvdata(getSockfd(),buffer+len,256);
	if(rbytes <= 0){
		LOG(DEBUG,"connectsocket close:%d",getSockfd());
		close();
		return false;
	}
	else{
		len+=rbytes;
		if(isIdStatus(buffer,len)){
			len = 0;
			return true;
		}
		buffer_handle();
	}
	return true;	
}