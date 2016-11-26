#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct node 
{
	int v;
	struct node *left;
	struct node *right;
};

void 
insert_node(struct node **root, int v)
{
	struct node *node = *root;
	if(NULL == node)
	{
		node = (struct node *)malloc(sizeof(struct node));
		memset(node, '\0', sizeof(struct node));
		node->v = v;
		*root = node;
		return ;
	}
	if(node->v > v)
	{
		return insert_node(&node->left, v);
	}
	return insert_node(&node->right, v);
}

void
display_tree(struct node *root)
{
	if(NULL == root)
	{
		return ;
	}
	display_tree(root->left);
	printf("%-3d", root->v);
	display_tree(root->right);
}

void
destroy_tree(struct node *root)
{
	if(NULL == root)
	{
		return ;
	}
	destroy_tree(root->left);
	destroy_tree(root->right);
	free(root);
}

struct node *
remove_node_aux(struct node **root)
{
	struct node *node = *root;
	if(NULL == node)
	{
		return NULL;
	}
	if(NULL == node->left)
	{
		*root = node->right;
		return node;
	}
	return remove_node_aux(&node->left);
}

void
remove_node(struct node **root, int a)
{
	struct node *node = *root;
	if(NULL == node)
	{
		return ;
	}
	if(node->v > a)
	{
		return remove_node(&node->left, a);
	}
	if(node->v < a)
	{
		return remove_node(&node->right, a);
	}
	struct node *r = remove_node_aux(&node->right);
	if(NULL == r)
	{
		*root = node->left;
	}
	else
	{
		r->left = node->left;
		r->right = node->right;
		*root = r;
	}
	free(node);
}

int 
main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("usage error\n");
		return 0;
	}
	int a[] = {32, 12, 2, 5, 10, 43, 123, 54, 90, 81, 98};
#define N sizeof(a) / sizeof(a[0])
	int i = 0;
	struct node *root = NULL;
	for(i = 0; i < N; ++i)
	{
		insert_node(&root, a[i]);
	}
	display_tree(root);
	printf("\n");
	remove_node(&root, atoi(argv[1]));
	display_tree(root);
	printf("\n");
	destroy_tree(root);
	return 0;
}
