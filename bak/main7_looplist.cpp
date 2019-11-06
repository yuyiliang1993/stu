#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//判断链表是否有环

/**
*方法1:使用标记 O(n)
*方法2:双指针 O(N+K) //K表示环的长度
*方法3:哈希表速度是 O(n)O(1)
*/
typedef struct ListNode{
	int cnt;
	int data;
	struct ListNode *next;
}*List,ListNode_t;

ListNode_t *initListNode(int data){
	ListNode_t*p = (List) malloc(sizeof(ListNode_t));
	if(p == NULL){
		perror("mallco");
		exit(1);
	}
	p->data = data;
	p->next = NULL;
	p->cnt = 0;
	return p;
}

List createList(int nums){
	ListNode_t* head = initListNode(-1);
	ListNode_t* p = NULL;
	ListNode_t* tail = head;
	int i = 0;
	int first = 0;
	while(i < nums){
		p = initListNode(i);
		tail->next = p;
		tail = p; 
		++i;
	}
	tail->next = head->next->next;
	return head;
}

void showList(List l){
	printf("show list:\n");
	ListNode_t *p = l;
	for(;p!=NULL;p=p->next){
		printf("%d\n",p->data);
	}
}

bool isLoopList_v1(ListNode_t* lst)
{
	ListNode_t *p1 = lst;
	for(;p1!=NULL;p1=p1->next){
		if(p1->cnt == 1){
			return true;
		}
		p1->cnt = 1;
	}
	return false;
}

bool isLoopList_v2(ListNode_t* lst)
{
	ListNode_t *p1 = lst;
	ListNode_t *p2 = lst->next;
	while(p1 != p2){
		if(p1 == NULL || p2==NULL){
			return false;
		}
		
		p2=p2->next;
		if(p2 == NULL){
			return false;
		}
		p2=p2->next;
		p1=p1->next;
	}
	return true;
}


int main(int argc,char *argv[])
{

	List l= createList(10);
	printf("is loop list:%d\n",isLoopList_v1(l));
	printf("is loop list:%d\n",isLoopList_v2(l));
	return 0;
}