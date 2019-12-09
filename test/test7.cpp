#include<iostream>
#include<string>
#include<cstring>
#include<exception>
class MyString{
public:
		MyString(){
			m_ptr = new char[1];
			m_ptr[0] = '\0';
		}
		MyString(const char*str);
		MyString(const MyString &rhs);
		MyString &operator =(const MyString &rhs);
		const char *getstr(){
			return m_ptr;
		}
		
		friend std::istream& operator>>(std::istream &is, MyString &str);//输入
		friend std::ostream& operator<<(std::ostream &os, MyString &str);//输出

		~MyString(){
			if(m_ptr)
				delete[] m_ptr;
			m_ptr = 0;
		}
private:
	char *m_ptr; 
};


MyString::MyString(const char*str){
	std::size_t len;
	if(str == NULL){
		m_ptr = new char[1];
		m_ptr[0] = '\0';
	}else
	 	len = strlen(str);
	m_ptr = new char[len+1];
	strcpy(m_ptr,str);
	m_ptr[len] = '\0';
//	throw new std::exception("qweqw");
}


MyString::MyString(const MyString & rhs){
	std::size_t len = strlen(rhs.m_ptr);
	std::cout<<"string size:"<<strlen(rhs.m_ptr)<<std::endl;
	this->m_ptr = new char[len+1];
	strcpy(this->m_ptr,rhs.m_ptr);
}


MyString &MyString::operator=(const MyString &rhs){
	if(this == &rhs)
		return *this;
	MyString tmp(rhs);
	char *ptr = tmp.m_ptr;
	tmp.m_ptr = this->m_ptr;
	this->m_ptr = ptr;;
	return *this;
}

#include<mutex>

std::mutex mt;
class Singletion{
public:
	static Singletion *getstance();
	
private:
	Singletion()=default;
	Singletion(const Singletion&rhs)=delete;
	Singletion&operator=(const Singletion&rhs)=delete;
	static Singletion *m_ptr;
	
};


Singletion *Singletion::m_ptr=0;

Singletion *Singletion::getstance(){
	static Singletion value;
	return &value;
}



int main(int argc,char*argv[])
{
	
	Singletion *ptr = Singletion::getstance();	
}



int  sock_read(int sock,char*buf,int size){
	do{
		int n =read(sock,buf,size);
		
	}while();
}

