#include <iostream>
#include <unordered_set>
#include <vector>

/*
题目要求
给定一个整数数组 nums 和一个目标值 target，请你在该数组中找出和为目标值的那 两个 整数，并返回他们的数组下标。

你可以假设每种输入只会对应一个答案。但是，你不能重复利用这个数组中同样的元素。
*/

/*
*思路,使用loop + hash表 + c++11
*/

/*
*编译
*g++ main0012.cpp -std=c++11
*/

std::vector<int> twosum(std::vector<int>&nums,int target)
{
	std::vector<int> tmp;
	int n = 0;
	std::unordered_set<int> hset;
	for(auto p=nums.begin();p!=nums.end();++p){
		n = target-*p;
		if(hset.find(n) != hset.end()){
			tmp.push_back(*p);
			tmp.push_back(n);
		}
		hset.insert(*p);
	}
	return tmp;
}



int main(int argc,char*argv[])
{
	std::unordered_set<int> hset;	
	std::vector<int> nums={1,2,3,4,5,6,7,8};
	std::vector<int>sum_vec= twosum(nums,8);
	for(auto p=sum_vec.begin();p!=sum_vec.end();++p){
		std::cout<<"num:"<<*p<<std::endl;
	}
}
