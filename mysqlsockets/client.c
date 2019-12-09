#include "client.h"
#include "sqlhandle.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "myzlog.h"
#include <string>
using namespace std;
extern SqlWork sql;//in sqlsocks.c
std::string tServerIdnums="ts123456";
Client::Client(const int&bufsize):cbuf(bufsize)
{
	fd = -1;
	idnum.clear();
	memset(&addr,0,sizeof(addr));
}

Client::Client()
{
	fd = -1;
	idnum.clear();
	memset(&addr,0,sizeof(addr));
}
bool Client::operator==(const Client & obj){
	return (fd == obj.fd && idnum==obj.idnum );
}

Client&Client::operator=(const Client &obj)
{
	if(this== &obj){
		return *this;
	}
	fd = obj.fd;
	addr = obj.addr;
	cbuf = obj.cbuf;
	idnum = obj.idnum;
	return *this;
}

Client::Client(const Client&obj)
{
	fd = obj.fd;
	addr = obj.addr;
	cbuf = obj.cbuf;
	idnum = obj.idnum;
}
void Client::init()
{
	fd = -1;
	cbuf.offetLens = 0;
	memset(cbuf.buffer,0,cbuf.totalLens);
	idnum.clear();
	memset(&addr,0,sizeof(addr));
}


bool string_to_json(const char*str,Json::Value &val)
{
	 Json::Reader reader;
	 if(reader.parse(str,val)){ 
		 return true; 
	 }
	 return false;
}
void showHex(const char *buffer,int len);

int sendJson(int fd,Json::Value root)
{
	Json::FastWriter w;
	std::string str =w.write(root);
	str+="\r\n";
//	LOG(DEBUG,"send:%s",str.c_str());
//	showHex(str.c_str(), str.length());
	//LOG(DEBUG,"send:%s",showHex(str.c_str(), str.length()));
	return senddata(fd,str.c_str(),str.length());
}

static void retLoginStatus(int fd,bool status)
{ 
	 Json::Value val; 
	 val["head"] = "SW"; 
	 val["type"] = "ZCFK";//注册信息 
	 val["content"]["status"] = status; 
	 sendJson(fd,val);
 }



ManageClients::ManageClients(const int &clientnums)
{
	clientsum = clientnums;
	clients = new Client[clientsum];
}

ManageClients::~ManageClients()
{
	delete[]clients;
}

int ManageClients::GetClientIndex(int fd)
{
	for(int i = 0;i<clientsum;++i){
		if(clients[i].fd == fd)
			return i;
	}
	return (-1);
}

bool ManageClients::AddClient(Client&cli)
{
	if(GetClientIndex(cli.fd)>=0){
		cerr<<"Err:cleint exists"<<endl;
		return false;
	}
	int i = GetClientIndex(-1);
	if(i >= 0){
		clients[i] = cli;
		return true;
	}
	cerr<<"Err:clients is full"<<endl;
	return false;
}

bool ManageClients::DelClient(Client &cli)
{
	if(cli.fd >= 0){
		::close(cli.fd);
	}
	cli.init();
}

bool ManageClients::DelClient(int index)
{
	return index>=0?DelClient(clients[index]):false; 
}

bool ManageClients::ReadClient(Client &cli)
{
	char * const buffer = cli.cbuf.buffer;
	int &len = cli.cbuf.offetLens;
	int maxlen = cli.cbuf.totalLens;
	if(len+256 > maxlen) // 存在丢数据
		len = 0;
	int rbytes = ::recvdata(cli.fd,buffer+len,256);
	if(rbytes <= 0){
		LOG(DEBUG,"client closed:%d,%s",cli.fd,cli.idnum.c_str());
		return false;
	}
	else{
		len+=rbytes;
		client_buffer_handle(cli);
	}
	return true;
}

bool ManageClients::ReadClient(int index)
{
	return index>=0?ReadClient(clients[index]):false;
}

Client &ManageClients::GetClient(int fd)
{
	int index = GetClientIndex(fd);
	return clients[index];
}

int ManageClients::sendToClient(std::string idnums,const Json::Value &root) const {
	for(int i = 0;i<clientsum;++i){
		if(clients[i].idnum == idnums){
			return (::sendJson(clients[i].fd,root));
		}
	}
	return 0;
}


bool ManageClients::IsClientLogin(Json::Value &root,Client&client)
{
	if(root["type"].isNull()){
		return false;
	}
	if(root["content"].isNull()){
		return false;
	}
	if(root["type"].isString() && root["type"] == "ZCXX"){
		if(root["content"]["idnums"].isNull()){
			return false;
		}
		if(root["content"]["idnums"].isString()){
	 		std::string idnum = root["content"]["idnums"].asString();
 			if(idnum.size() == 0){
				 return false;
		 	}
			LOG(DEBUG,"[%s][%d]new idnum:%s",inet_ntoa(client.addr.sin_addr),ntohs(client.addr.sin_port),idnum.c_str());
			existedLogin(idnum,client);
		 	client.idnum = idnum;
		 	return true;
		}
	}
	return false;
}

bool ManageClients::existedLogin(const std::string&idnum,Client &client)
{
	for(int i =0;i<clientsum;++i)
	{
		if(clients[i].idnum == idnum){
			if(clients[i].fd != client.fd){
				cout<<"old client closed:"<<clients[i].fd<<endl;
				DelClient(clients[i]);
			}
		}
	}
}

bool ManageClients::data_source_dispatch(Json::Value &root,Client &client)
{
	if(IsClientLogin(root,client)){
		retLoginStatus(client.fd, 1);
		return true;
	}
	if(client.idnum.size() <= 0){
		retLoginStatus(client.fd,0);
		return true;
	}
	static SqlWork sqlwork;
	if(client.idnum.size() > 0){
		sqlwork.query_analysis(root,client.fd,*this);
		return true;
	}
	return true;
}

bool ManageClients::extractJson(const char *data,Client &client)
{
	Json::Value root;
	if(stringToJson(data,root)){
		data_source_dispatch(root, client);
		return true;
	}
	return false;
}

bool ManageClients::stringToJson(const char*str,Json::Value &val)
{
	 Json::Reader reader;
	 if(reader.parse(str,val)){ 
		 return true; 
	 }
	 return false;
}

#if 0
bool ManageClients::data_protocol_analysis(const char *data,const int &dlen,Client &client)
{
	static int bufflen = 1024*100;
	static char *buffer = (char*)malloc(sizeof(char)*bufflen);//固定100k内存
	if(dlen > bufflen){
		if(buffer)
			free(buffer);
		buffer = (char*)malloc(sizeof(char)*dlen);
		bufflen = dlen;
		LOG(DEBUG,"buffersize update:%d",dlen);
	}

	if(NULL == buffer){
		LOG(FATAL,"malloc buf failed:%s",::strerror(errno));
		return false;
	}
	memcpy(buffer,data,dlen);
	Json::Value root;
	if(string_to_json(data,root)){
		LOG(DEBUG,"recv json:%s",root.toStyledString().c_str());
		data_source_dispatch(root, client);
	}
	return true;
}
#endif

bool ManageClients::client_buffer_handle(Client &client)
{
	char *const buffer = client.cbuf.buffer;
	int &offsetlen = client.cbuf.offetLens;
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
			Json::Value root;
			if(stringToJson(p,root)){
				data_source_dispatch(root, client);
			}
		}
		p = pstr;
	}
	if(packnums > 0){
		offsetlen = 0;
	}
}


