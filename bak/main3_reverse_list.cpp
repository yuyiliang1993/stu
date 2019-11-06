#include <stdio.h>
#include <stdlib.h>

/*
*二叉树的逆转，主要用到了前后指针
*/

struct Node{
	int data;
	struct Node *next;
};

typedef struct Node Node_t;
typedef Node_t* List;

List createList(int num)
{
	Node_t *head = (Node_t *)malloc(sizeof(Node_t));
	head->next =NULL;
	head->data = 0;
	Node_t *tmp_head =head;
	int i = 0;
	for(i=0;i<num;++i)
	{
		Node_t *p = (Node_t *)malloc(sizeof(Node_t));
		p->next = NULL;
		p->data = i+1;
		head->next = p;
		head = p;
	}
	return tmp_head;
}


Node_t* reverseList(List lst)
{
	Node_t *p = lst;
	Node_t *q,*r;
	q=r=NULL;
	while(p){
		q=p->next;
		p->next=r;
		r=p;
		p=q;
	}
	return r;
}


Node_t* reverseList2(Node_t *head)
{
	if(head==NULL||head->next==NULL){
		return head;
	}
	
	Node_t * last =  head->next;
	Node_t * cur =  last->next;
	Node_t * tmp = NULL;

	while(cur){
		tmp= cur->next;
		cur->next = head->next;
		head->next = cur;
		cur = tmp;
	}
	
	tmp = head->next;
	head ->next = NULL;
	last->next = head;
	return tmp;
}

void printList(List l)
{
	Node_t *p = l;
	printf("list:\n");
	while(p){
		printf("%d ",p->data);
		p=p->next;
	}
	printf("\n");
}

int main(int argc,char **argv)
{
	List l = createList(100);
	printf("reverse start:\n");
	printList(l);
	l = reverseList2(l);
	printf("reverse end:\n");
	printList(l);
}

