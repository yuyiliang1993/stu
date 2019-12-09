#include <stdio.h>
#include <stdlib.h>

static int get_max(int arr[],int n){
	int max = arr[0];
	int i = 0;
	for(i = 1;i<n;++i){
		if(arr[i] > max){
			max = arr[i];
		}
	}
	return max;
}

static void countSort(int arr[],int n,int exp)
{
	int tmpsave[10][n];
	int count[10]={0};
	int i = 0;
	int index = 0;
	for(i=0;i<n;++i){
		index = (arr[i]/exp)%10;
		tmpsave[index][count[index]++] = arr[i];
	}

	index = 0;
	int j = 0;
	for(i = 0;i<10;++i){
		for(j =0;j<count[i];++j){
			arr[index++] = tmpsave[i][j];
		}
	}

}


void radixSort(int arr[],int n)
{
	printf("running radixSort...\n");
	int max = get_max(arr, n);
	int exp = 0;
	for(exp = 1;max/exp > 0;exp*=10){
		countSort(arr,n, exp);
	}	
}



int main(int argc,char *argv[])
{
	//int array[]={1,122,356,27,8,987,0,9999,311111,88};
	int array[]={100,2,10,11,9,0,1,100,2,99,1,3,5,4,88,12,34,55,13,88,77,66,111,444,222};
	int n = sizeof(array)/sizeof(array[0]);
	

	printf("before sort:");
	int i = 0;
	while(i<n){
		printf(" %d",array[i++]);
	}
	printf("\n");

	radixSort(array,n);

	printf("after sort:");
	
	i = 0;
	while(i<n){
		printf(" %d",array[i++]);
	}
	printf("\n");
	
}
