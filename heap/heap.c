#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct node 
{
	int value;
	struct node *rchild;
	struct node *lchild;
};

void
InsertHeap(struct node **node, int value)
{
	if(*node == NULL)
	{
		struct node *n = (struct node *)malloc(sizeof(struct node));
		n->rchild = n->lchild = NULL;
		n->value = value;
		*node = n;
		return;
	}
	if((*node)->value > value)
	{
		InsertHeap(&(*node)->lchild, value);
	}
	else
	{
		InsertHeap(&(*node)->rchild, value);
	}
}

void 
PrintHeap(struct node *node)
{
	if(node == NULL)
	{
		return;
	}
	if(node->lchild)
	{
		PrintHeap(node->lchild);
	}
	printf("%d ", node->value);
	if(node->rchild)
	{
		PrintHeap(node->rchild);
	}
}

int
main()
{
	int a[] = {123,213,234,5,6,5467,76,89,4,3,43};
#define num sizeof(a) / sizeof(a[0])
	int i;
	struct node *heap = NULL;
	for(i = 0; i < num; i++)
	{
		InsertHeap(&heap, a[i]);
	}
	PrintHeap(heap);
	printf("\n");
	return 0;
}

