//#include<iostream>
#include <stdio.h>
typedef struct Node Node_t;
struct Node{
	void *data;
	Node_t *next;
};

struct List
{
	Node_t *Head;
	void (*push)(struct List *lst,void *data);
};


int i = 0;

class Test1{
public:
	static int m_value;
	static void print();
};

class Test2{
public:
	static void print();
};


void Test2::print()
{
	printf("Test2\n");
}


void Test1::print()
{
	printf("Test1\n");
}


int Test1::m_value = 100;

#include<iostream>
#include<string>

void pirnt(const int &str)
{
	std::cout<<str<<std::endl;
}


int main(int argc,char*argv[])
{
	
	Test1 t1;
	t1.print();
	printf("%d\n",t1.m_value);
	Test2::print();
	Test2 t2;
	pirnt(100);
}
