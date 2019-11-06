#include<iostream>
#include <cstdio>

/*交换函数*/
void swap1(int *a,int *b){
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

/**
*swap2和swap3针对同一个变量，会导致这个变量为0
*因为两个指针指向同一个变量。
*/
void swap2(int *a,int *b)
{	
	if(a == b){
		return ;
	}
	*a = *a ^ *b;
	*b = *a ^ *b; //*a ^  *b  ^  *b -> *a
	*a = *a ^ *b; //*a ^  *b  ^  *a -> *b
}

void swap3(int *a,int *b)
{
	if(a == b)
		return ;
	*a = *a + *b;
	*b = *a - *b;
	*a = *a - *b;
}


/*快速排序*/
//不稳定，时间复杂度O(logN) - O(N) 空间复杂度O(1)
void quicksort(int *a,int low,int high)
{

	int i = low;
	int j = high;
	int key = a[low];
	if(low >= high){
		return;
	}
	while(low < high){
		
		while(low<high && key <= a[high]){
			--high;
		}
		if(key > a[high]){
			swap2(&a[low], &a[high]);//交换最低位与最高位置得数据
			++low;
		}

		while(low <high && key >= a[low] ){
			++low; 
		}
		
		if(key < a[low]){
			swap2(&a[low], &a[high]);
			--high;
		}
	}
	
	quicksort(a,i,low-1);
	quicksort(a,low+1,j);
	
}


/*冒泡排序*/
//稳定，最坏O(N^2) 最好O(N) 空间复杂度O(1)
void maopaoSort(int *a,int size)
{
	int j,i;
	for(i=0;i<size;++i){
		for(j=i+1;j<size;++j){
			if(a[j] > a[i]){
				swap2(&a[j],&a[i]);
			}
		}
	}	
}


/*插入排序*/
//稳定，最坏O(N^2) 最好O(N)
void insertSort(int nums[],int size)
{
	int i,j;
	for(i=1;i<size;++i){
		int n = nums[i];
		j = i-1;
		while(j>=0&&n>=nums[j]){
			nums[j+1] = nums[j];
			--j;
		}
		nums[j+1] = n;
	}
}


/*选择排序*/
//不稳定，最坏O(N^2) 最好O(N)
void selectSort(int *nums,int size)
{
	int i = 0,j =0;
	int index = 0;
	for(i=0;i<size-1;++i){
		index = i;
		for(j=i+1;j<size;++j){
			if(nums[index] < nums[j]){
				index = j;
			}
		}
		swap2(&nums[index],&nums[i]);
	}
}


void show(int *a,int size)
{
	for(int i=0;i<size;++i){
		std::cout<<a[i]<<" ";
	}
	std::cout<<std::endl;
}


int main()
{
	int arra[] = {100,2,10,11,9,0,1,100,2,99,1,3,5,4,88,12,34,55,13,88,77,66,111,444,222};
	int size = sizeof(arra)/sizeof(arra[0]);
	std::cout<<"sort begin..."<<std::endl;
	show(arra, size);
	selectSort(arra,size);
	std::cout<<"sort finish"<<std::endl;
	show(arra, size);
}
