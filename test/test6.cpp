#include <stdio.h>
#include <stdlib.h>
typedef struct BTreeNode
{
    int _Value;
    struct BTreeNode* _Left;
    struct BTreeNode* _Right;
}BTreeNode;

BTreeNode* core(int* preStart,int* preEnd,int* inStart,int* inEnd)
{
//前序遍历的地一个元素就是根节点
    int rootValue = preStart[0];
    BTreeNode* root = (BTreeNode*)malloc(sizeof(BTreeNode));
    root->_Value = rootValue;
    root->_Left = root->_Right = NULL;
//在中序遍历序号中找到根节点的值
    int* rootIn = inStart;
    while(rootIn <= inEnd && *rootIn != rootValue) ++rootIn;

    int leftLen = rootIn - inStart;
    int* leftPreEnd = preStart + leftLen;

    if(leftLen>0)
    {
        root->_Left = core(preStart+1,leftPreEnd,inStart,inStart+leftLen-1);
    }
    if(leftLen < preEnd - preStart)
    {
        root->_Right = core(leftPreEnd+1,preEnd,rootIn+1,inEnd);
    }
    return root;
}

BTreeNode* construct(int* pre,int* in,int len)
{
    if(pre==NULL || in==NULL || len<=0) return 0;
    return core(pre,pre+len-1,in,in+len-1);
}

void preOrder(BTreeNode* root)
{
    if(root ==NULL) return;
    printf("%d ",root->_Value);
    preOrder(root->_Left);
    preOrder(root->_Right);
}

int main()
{
    int pre[] = {1,2,4,7,3,5,6,8};
    int in[] = {4,7,2,1,5,3,8,6};
    BTreeNode* root = NULL;
    root = construct(pre,in,8);
    preOrder(root);
	printf("\n");
    return 0;
}

