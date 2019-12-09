#include<iostream>
#include<unordered_map>
#include<list>

class LRUCache {
private:
	int capacity;
	std::list<std::pair<int,int>> m_list;
	std::unordered_map<int,std::list<std::pair<int,int>>::iterator> mp;
public:
	LRUCache(int capacity){
		this->capacity = capacity;
	}
	void put(int key,int value){
		if(mp.find(key) == mp.end()){
				if(m_list.size() == capacity){
					auto lastpair = m_list.back();
					mp.erase(lastpair.first);
					std::cout<<"pop last:"<<lastpair.second<<std::endl;
					m_list.pop_back();
				}
				m_list.push_front(std::make_pair(key,value));
				mp[key] = m_list.begin();
		}
		else{
			auto node = mp[key];
			auto pair = *node;
			m_list.erase(node);
			m_list.push_front(pair);
			mp[key] = m_list.begin();
		}
	}

	int get(int key){
		if(mp.find(key) != mp.end()){
			auto val = mp[key];
			std::pair<int,int>tmp=*mp[key];
			m_list.erase(mp[key]);
			m_list.push_front(tmp);
			mp[key] = m_list.begin();
			return mp[key]->second;
		}
		return -1;
	}	
};


int main(){
	LRUCache lru(3);
	lru.put(1, 10);	
	lru.put(2, 20);
	lru.put(3, 30);
	lru.put(4, 40);
	std::cout<<"get:"<<lru.get(1);
	std::cout<<std::endl;
}
