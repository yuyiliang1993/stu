#include <stdio.h>
//这个程序主要判断是大端还是小端
//如果是大端，a的值输出是0，小端则与b相同
union Data{
	int b;
	char a;
};

int main(){
	union Data v;
	v.b = 1;
	printf("b:%d\n",v.b);
	printf("a:%d\n",v.a);
}
