#include<iostream>
#include<map>
#include<set>
#include<string>
//#include<hash_set>
int main()
{
	hash_set set;
	std::map<int,std::string> m;
	m.insert(std::pair<int,std::string>(1,"qeqweq"));
	m.insert(std::pair<int,std::string>(2,"1000000"));
	m[2] = "qqqqqqqqqqqq";
		
	std::map<int,std::string>::iterator iter;
	for(iter=m.begin();iter!=m.end();iter++){
		std::cout<<iter->first<<":"<<iter->second<<std::endl;
	}

	std::map<std::string,std::string> mstr;
	mstr.insert(std::make_pair<std::string,std::string>("yl","nihao"));

	std::map<std::string,std::string>::iterator iter1;
	for(iter1=mstr.begin();iter1!=mstr.end();++iter1){
		std::cout<<iter1->first<<":"<<iter1->second<<std::endl;
	}
	
	std::cout<<mstr["xx"].size()<<std::endl;
	std::cout<<(iter1=mstr.find("yl"))->first<<std::endl;
	
}
