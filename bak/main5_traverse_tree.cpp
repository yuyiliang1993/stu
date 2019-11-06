#include<stdio.h>
#include<iostream>
#include<cstdlib>
/*
*创建二叉树以及遍历
*/

typedef char ElemType;
typedef struct BitNode{
	ElemType data;
	struct BitNode *lchild,*rchild;
}BitNode,*BitTree;


int getDataByInput(ElemType *data){
	std::cin>>data;
}

void createBitTree(BitTree *t)
{
	ElemType data;
	std::cin>>data;
	if(data == '$'){
		(*t) = NULL;
	}
	else {
		(*t) = (BitTree) malloc(sizeof(BitNode));
		(*t)->data = data;
		createBitTree(&((*t)->lchild));
		createBitTree(&((*t)->rchild));
	}
}


//根->左->右
void preOrderTree(BitTree t)
{
	if(t == NULL){
		return ;
	}
	std::cout<<t->data<<" ";
	preOrderTree(t->lchild);
	preOrderTree(t->rchild);
	
}

//左->根->右
void inOrderTree(BitTree t)
{
	if(t == NULL){
		return ;
	}
	inOrderTree(t->lchild);
	std::cout<<t->data<<" ";
	inOrderTree(t->rchild);
}

//左->右->根
void postOrderTree(BitTree t)
{
	if(t == NULL){
		return ;
	}

	inOrderTree(t->lchild);
	inOrderTree(t->rchild);
	std::cout<<t->data<<" ";
}


#include <stack>
void preOrderTreeByLoop(BitTree t)
{
	if(t==NULL)
		return ;

	std::stack<BitTree> s;
	BitTree p = t;
	while(p != NULL || !s.empty()){
		if(p != NULL ){
			s.push(p);
			std::cout<<p->data<<" ";
			p=p->lchild;
		}else {
			p=s.top();
			s.pop();
			p=p->rchild;
		}
	}
}


void inOrderTreeByLoop(BitTree t){
	if(t == NULL)
		return ;
	std::stack<BitTree> s;
	BitTree p = t;
	while(p || !s.empty()){

		if(p!=NULL){
			s.push(p);
			p=p->lchild;
		}
		else {
			p=s.top();
			s.pop();
			std::cout<<p->data<<" ";
			p=p->rchild;
		}
	}

}

typedef struct tmpBitTreeNode{
	BitTree bt;
	char tag;
}*tmpTreeBode;



void postOrderTreeByLoop(BitTree t){
	std::stack<tmpTreeBode> s;
	BitTree p = t;
	while(p ||!s.empty())
	{
		//把所有左节点放到stack中
		
		while(p!=NULL){
			tmpTreeBode pbt = (tmpTreeBode)malloc(sizeof(struct tmpBitTreeNode));
			pbt->bt = p;
			pbt->tag='L';
			s.push(pbt);
			p=p->lchild;
		}
		
		while(!s.empty() && (s.top())->tag=='R'){
			p= s.top()->bt;
			std::cout<<p->data<<" ";
			s.pop();
		}
		//把每一个左结点取出，取右节点
		if(!s.empty()){
			tmpTreeBode pbt = s.top();
			p=pbt->bt;
		//	std::cout<<p->data<<" ";
			p=p->rchild;
			pbt->tag ='R';			
		}
	
	}
}


//层次遍历
#include<queue>

void levelTraverse(BitTree t)
{
	std::queue<BitTree> q;
	q.push(t);
	BitTree p = 0;
	while(!q.empty()){
		p=q.front();
		q.pop();
		std::cout<<p->data<<" ";

		if(p->lchild){
			q.push(p->lchild);
		}
		if(p->rchild)
		{
			q.push(p->rchild);
		}
	}
}


int main(int argc,char **argv)
{
	BitTree t;
	createBitTree(&t);
	std::cout<<"start order:";
	preOrderTree(t);
	std::cout<<std::endl;
}
