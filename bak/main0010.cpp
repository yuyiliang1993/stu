//最长回文实现

#include<stdio.h>
#include<string>
#include<iostream>
int expandAroundCenter(std::string s, int left, int right) {
    int L = left, R = right;
    while (L >= 0 && R < s.length() && s[L] == s[R] ) {
        L--;
        R++;
    }
    return R - L - 1;
}

//起始位置和结束位置

std::string longestPalindrome(std::string s) {
    if (s.size()==0 || s.length() < 1){
		return "";
	}
    int start = 0, end = 0;
	int i;
	for(i = 0; i < s.length(); i++) {
        int len1 = expandAroundCenter(s, i, i);
        int len2 = expandAroundCenter(s, i, i + 1);
        int len =(len1>len2)?len1:len2;
        if (len > end - start) {
            start = i - (len - 1) / 2;
            end = i + len / 2;
        }
    }
    return s.substr(start, end+1);
}


int main(int argc,char*argv[])
{
	std::string srcstr("bacabacb");
	std::cout<<"result:"<<longestPalindrome(srcstr)<<std::endl;
}
