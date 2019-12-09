#include<iostream>
#include<unordered_set>
#include<set>

void test(){
	std::unordered_set<int>s;
	s.insert(1);
	s.insert(100);
	s.insert(1000);
	s.insert(99999);
	auto iter = s.find(99999);
	if(iter == s.end()){
		std::cout<<"not find 99999"<<std::endl;
	}else{
		std::cout<<"find:"<<*iter<<std::endl;
	}

	for(auto i =s.begin();i!=s.end();++i){
		std::cout<<"loop:"<<*i<<std::endl;
	}
}

void test2()
{
	std::set<int>s;
	s.insert(1);
	s.insert(100);
	s.insert(1000);
	s.insert(99999);
	auto iter = s.find(99999);
	if(iter == s.end()){
		std::cout<<"not find 99999"<<std::endl;
	}else{
		std::cout<<"find:"<<*iter<<std::endl;
	}

	std::cout<<"size:"<<s.size()<<std::endl;
	
	
	for(auto i =s.begin();i!=s.end();++i){
		std::cout<<"loop:"<<*i<<std::endl;
		s.erase(i);
	}

	std::cout<<"size:"<<s.size()<<std::endl;
}


#include<vector>
void test3(){
	std::vector<int> v;
	v.push_back(1);
	v.push_back(10);
	v.push_back(100);
	v.push_back(1000);
	std::cout<<"size:"<<v.size()<<std::endl;
		
	for(auto i =v.begin();i!=v.end();++i){
		std::cout<<"loop:"<<*i<<std::endl;
		v.erase(i);
		--i;
	}
	
	std::cout<<"size:"<<v.size()<<std::endl;
}

#include<stack>

void test4(){
	std::stack<int> v;
	v.push(1);
	v.push(10);
	v.push(100);
	v.push(1000);
	std::cout<<"size:"<<v.size()<<std::endl;		
	while(!v.empty()){
		std::cout<<"stack:"<<v.top()<<std::endl;		
		v.pop();
	}
	std::cout<<"size:"<<v.size()<<std::endl;
}

#include<queue>

void test5(){
	std::queue<int> v;
	v.push(1);
	v.push(10);
	v.push(100);
	v.push(1000);
	std::cout<<"size:"<<v.size()<<std::endl;		
	while(!v.empty()){
		std::cout<<"queue:"<<v.front()<<std::endl;		
		v.pop();
	}
	std::cout<<"size:"<<v.size()<<std::endl;
}

#include<map>
#include<string>
#include<utility>
void test6()
{
	std::map<int,std::string> v;
	v.insert(std::pair<int,std::string>(1,"qwer"));
	
	std::cout<<"size:"<<v.size()<<std::endl;

	auto iter = v.find(2);
	if(iter != v.end()){
		std::cout<<"map find:"<<iter->first<<"->"<<iter->second<<std::endl;

	}else
		std::cout<<"not find"<<std::endl;
	
	for(auto i=v.begin();i!=v.end();++i){
		std::cout<<"map:"<<i->first<<"->"<<i->second<<std::endl;
		v.erase(i);
	}
	
	std::cout<<"size:"<<v.size()<<std::endl;
}


int main(int argc,char *argv[])
{
	test6();
}
