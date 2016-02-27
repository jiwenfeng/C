#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node
{
	struct node *left;
	struct node *right;
	int v;
	int h;
};

#define Max(a, b) ((a) > (b) ? (a) : (b))

static int
height(struct node *n)
{
	return n == NULL ? -1 : n->h;
}

static void
LRoration(struct node **root)
{
	struct node *r = *root;
	struct node *c = r->left;
	r->left = c->right;
	r->h = Max(height(r->left), height(r->right)) + 1;
	c->right = r;
	c->h = Max(height(c->left), height(c->right)) + 1;
	*root = c;
}

static void
RRoration(struct node **root)
{
	struct node *r = *root;
	struct node *c = r->right;
	r->right = c->left;
	r->h = Max(height(r->left), height(r->right)) + 1;
	c->left = r;
	c->h = Max(height(c->left), height(c->right)) + 1;
	*root = c;
}

static void
insert(struct node **root, int n)
{
	struct node *r = *root;
	if(r == NULL)
	{
		*root = (struct node *)malloc(sizeof(struct node));
		(*root)->h = 0;
		(*root)->left = (*root)->right = NULL;
		(*root)->v = n;
		return;
	}
	else if(r->v > n)
	{
		insert(&r->left, n);
		r->h = Max(height(r->left), height(r->right)) + 1;
		if(height(r->left) - height(r->right) >= 2)
		{
			if(r->left->v < n) // LR
			{
				RRoration(&r->left);
				LRoration(root);
			}
			else // LL
			{
				LRoration(root);
			}
		}
	}
	else if(r->v < n)
	{
		insert(&r->right, n);
		r->h = Max(height(r->left), height(r->right)) + 1;
		if(height(r->right) - height(r->left) >= 2)
		{
			if(r->right->v > n) // RL
			{
				LRoration(&r->right);
				RRoration(root);
			}
			else // RR
			{
				RRoration(root);
			}
		}
	}
	else
	{
		printf("%d is exist in the AVL Tree!\n", n);
		return;
	}
}


static void
display(struct node *root)
{
	if(root == NULL)
	{
		return;
	}
	printf("%d(%d, %d)\n", root->v, root->left ? root->left->v : -1, root->right ? root->right->v : -1);
	display(root->left);
	display(root->right);
}

static void
destroy(struct node *root)
{
	if(root == NULL)
	{
		return ;
	}
	destroy(root->left);
	destroy(root->right);
	free(root);
}

static int
max_value(struct node *root)
{
	if(root == NULL)
	{
		return -1;
	}
	if(root->right == NULL)
	{
		return root->v;
	}
	return max_value(root->right);
}

static int
min_value(struct node *root)
{
	if(root == NULL)
	{
		return -1;
	}
	else if(root->left == NULL)
	{
		return root->v;
	}
	return min_value(root->left);
}

int 
main()
{
	int a[] = {32, 12, 23, 21, 59, 90, 68, 80, 41, 72};
#define N (sizeof(a) / sizeof(a[0]))
	int i;
	struct node *root = NULL;
	for(i = 0; i < 10; ++i)
	{
		insert(&root, a[i]);
	}
	display(root);
	printf("Min:%d, Max:%d\n", min_value(root), max_value(root));
	destroy(root);
	return 0;
}

