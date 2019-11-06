#include <iostream>
#include <set>
#include <vector>
#include <unordered_set>

//关于哈希set和map的使用
//pair的使用

#include <unordered_map>
typedef int Type;
bool findDuplicates(std::vector<Type>& keys)
{
    std::unordered_set<Type> hashset;

	for(Type key:keys){
		std::cout<<"key:"<<key<<std::endl;
		if(hashset.count(key) > 0)
			return true;
		hashset.insert(key);
	}
	
	return false;
}

bool findDuplicates(const std::vector<Type>& keys)
{
    std::unordered_map<Type,int> hashmap;
	int count = 0;
	for(Type key:keys){
		std::cout<<"map key:"<<key<<std::endl;
		if(hashmap.count(key) > 0)
			return true;
		hashmap.insert(std::make_pair(key,count++));
	}
	return false;
}


int main(int argc,char *argv[])
{
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	vec.push_back(3);
	const std::vector<int> &a = vec;
	std::cout<<findDuplicates(a);
	std::cout<<std::endl;
	
}
