#include<iostream>
#include<cstdlib>
#include<cstdio>
#include<unordered_set>

/*
*题目
*给定一个链表，要求判断是否有环
*/


/**
*方法1:哈希表速度是 O(n)O(1)
*方法2:双指针 O(N+K) //K表示环的长度
*方法3:使用标记 O(n)
*/

struct ListNode{
	int data;
	int flag;
	struct ListNode *next;
};

typedef struct ListNode ListNode_t; 

//hash法
bool isLoopList_v1(ListNode_t*head){
	if(head == NULL){
		return false;
	}
	std::unordered_set<ListNode_t*> hset;//hash set
	for(ListNode_t* p= head;p!=0;p=p->next){
		if(hset.find(p) != hset.end()){
			return true;
		}
		hset.insert(p);
	}
	return false;
}

//快指针法
static bool isLoopList_v2(ListNode_t* lst)
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

//标记法，缺点是需要在节点额外添加一个标记位
static bool isLoopList_v3(ListNode_t *lst)
{
	ListNode_t *p1 = lst;
	for(;p1!=NULL;p1=p1->next){
		if(p1->flag == 1){
			return true;
		}
		p1->flag = 1;
	}
	return false;	
}


//创建一个节点
static ListNode_t *initListNode(int data)
{
	ListNode_t*p = new ListNode;
	p->data = data;
	p->next = NULL;
	return p;
}

//创建一个带环得链表
static ListNode_t*createLoopList(int nums){
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

//创建一个非带环的链表
static ListNode_t*createNoLoopList(int nums){
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
	return head;
}

//创建链表
ListNode_t *createList(ListNode_t*(*fun)(int nums),int nums){
	return fun(nums);
}

void showList(ListNode_t*l){
	printf("show list:\n");
	ListNode_t *p = l;
	for(;p!=NULL;p=p->next){
		printf("%d\n",p->data);
	}
}

int main(void){
	ListNode_t *l1 = createList(createLoopList,10);
	ListNode_t *l2 = createList(createNoLoopList,10);
	printf("is loop list v1:%d\n",isLoopList_v1(l1));
	printf("is loop list v2:%d\n",isLoopList_v2(l2));
}
