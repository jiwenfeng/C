#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct node 
{
	int value;
	int height;
	struct node *lchild;
	struct node *rchild;
	struct node *father;
};

#define FATHER(x) (x)->father

#define GRANDPA(x) (FATHER(FATHER(x)))

#define HEIGHT(n) ((n) == NULL ? -1 : (n)->height)

#define MAX(i1, i2) ((i1) > (i2) ? (i1) : (i2))

#define SWAP(i1, i2) do { int v = (i1)->value; (i1)->value = (i2)->value; (i2)->value = v; } while(0)

void
avl_l_rotation(struct node *n)
{
	struct node *f = FATHER(n);
	struct node *r = n->rchild;
	n->rchild = r->lchild;
	if(r->lchild)
	{
		FATHER(r->lchild) = n;
	}
	FATHER(r) = f;
	r->lchild = n;
	FATHER(n) = r;
	if(f)
	{
		if(f->lchild == n)
		{
			f->lchild = r;
		}
		else
		{
			f->rchild = r;
		}
	}
	n->height = MAX(HEIGHT(n->lchild), HEIGHT(n->rchild)) + 1;
	r->height = MAX(HEIGHT(r->lchild), HEIGHT(r->rchild)) + 1;
}

void
avl_r_rotation(struct node *n)
{
	struct node *f = FATHER(n);
	struct node *l = n->lchild;
	n->lchild = l->rchild;
	if(l->rchild)
	{
		FATHER(l->rchild) = n;
	}
	FATHER(l) = f;
	l->rchild = n;
	FATHER(n) = l;
	if(f)
	{
		if(f->lchild == n)
		{
			f->lchild = l;
		}
		else
		{
			f->rchild = l;
		}
	}
	n->height = MAX(HEIGHT(n->lchild), HEIGHT(n->rchild)) + 1;
	l->height = MAX(HEIGHT(l->lchild), HEIGHT(l->rchild)) + 1;
}

static void
avl_insert_fixup(struct node **root, struct node *n)
{
	while(n != *root && GRANDPA(n) != NULL)
	{
		if(HEIGHT(GRANDPA(n)->lchild) - HEIGHT(GRANDPA(n)->rchild) >= 2)		// L
		{
			if(FATHER(n)->rchild == n)	// L
			{
				n = FATHER(n);
				avl_l_rotation(n);
			}
			avl_r_rotation(GRANDPA(n));
			n = FATHER(n);
		}
		else
		{
			if(HEIGHT(GRANDPA(n)->rchild) - HEIGHT(GRANDPA(n)->lchild) >= 2) // R
			{
				if(FATHER(n)->lchild == n)
				{
					n = FATHER(n);
					avl_r_rotation(n);
				}
				avl_l_rotation(GRANDPA(n));
				n = FATHER(n);
			}
			else
			{
				n = FATHER(n);
			}
		}
		if(FATHER(n) == NULL)
		{
			*root = n;
			break;
		}
	}
}

void
avl_insert(struct node **root, int v)
{
	struct node **itr = root;
	struct node *p = NULL;
	while(*itr != NULL)
	{
		if((*itr)->value == v)
		{
			return ;
		}
		p = *itr;
		itr = (*itr)->value > v ? &(*itr)->lchild : &(*itr)->rchild;
	}
	struct node *n = (struct node *)malloc(sizeof(struct node));
	n->value = v;
	n->height = 0;
	n->lchild = n->rchild = n->father = NULL;
	*itr = n;
	FATHER(n) = p;
	while(p != NULL)
	{
		p->height = MAX(HEIGHT(p->lchild), HEIGHT(p->rchild)) + 1;
		p = FATHER(p);
	}
	avl_insert_fixup(root, n);
}

static void
avl_delete_fixup(struct node **root, struct node *n)
{
	while(abs(HEIGHT(n->lchild) - HEIGHT(n->rchild)) >= 2)
	{
		if(HEIGHT(n->lchild) - HEIGHT(n->rchild) >= 2)
		{
			if(n->lchild->rchild != NULL)
			{
				avl_l_rotation(n->lchild);
			}
			avl_r_rotation(n);
			n = FATHER(n);
		}
		else
		{
			if(HEIGHT(n->rchild) - HEIGHT(n->lchild) >= 2)
			{
				if(n->rchild->lchild != NULL)
				{
					avl_r_rotation(n->rchild);
				}
				avl_l_rotation(n);
				n = FATHER(n);
			}
			else
			{
				n = FATHER(n);
			}
		}
		if(FATHER(n) == NULL)
		{
			*root = n;
			break;
		}
	}
}

void 
avl_delete(struct node **root, int v)
{
	struct node *n = *root;
	for(; n != NULL && n->value != v ; n = n->value > v ? n->lchild : n->rchild)
	{}
	if(n == NULL)
	{
		return;
	}
	struct node **itr = &n->rchild;
	for(; *itr != NULL && (*itr)->lchild; itr = &(*itr)->lchild)
	{}
	struct node *x = *itr;
	if(x == NULL)
	{
		x = n;
		if(FATHER(n)->lchild == n)
		{
			FATHER(n)->lchild = NULL;
		}
		else
		{
			FATHER(n)->rchild = NULL;
		}
	}
	else
	{
		SWAP(n, x);
		if(x->rchild)
		{
			FATHER(x->rchild) = FATHER(x);
		}
		*itr = x->rchild;
	}
	avl_delete_fixup(root, FATHER(x));
	free(x);
}

void 
avl_debug(struct node *root)
{
	if(root == NULL)
	{
		return ;
	}
	printf("%d:%d %d\n", root->value, root->lchild ? root->lchild->value : -1, root->rchild ? root->rchild->value : -1);
	avl_debug(root->lchild);
	avl_debug(root->rchild);
}

void 
avl_destroy(struct node *root)
{
	if(root == NULL)
	{
		return;
	}
	avl_destroy(root->lchild);
	avl_destroy(root->rchild);
	free(root);
}

int
main(int argc, char *argv[])
{
	int a[] = {32, 12, 23, 21, 59, 90, 68, 80, 41, 72, 34, 89, 21, 97, 56, 70, 10, 13};
#define N (sizeof(a) / sizeof(a[0]))
	int i = 0;

	struct node *root = NULL;
	for(i = 0; i < N; ++i)
	{
		avl_insert(&root, a[i]);
	}
	avl_debug(root);
	if(argc == 2)
	{
		avl_delete(&root, atoi(argv[1]));
	}
	printf("---------------------------------\n");
	avl_debug(root);
	avl_destroy(root);
	return 0;
}
