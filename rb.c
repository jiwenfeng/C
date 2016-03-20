#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

struct node 
{
	int v;
	int color;
	struct node *lchild;
	struct node *rchild;
	struct node *parent;
};

#define RED 0x01
#define BLACK 0x02

#define FATHER(x) ((x)->parent)

#define COLOR(x) ((x) ? ((x)->color) : BLACK)

#define GRANDPA(x) FATHER(FATHER(x))

#define UNCLE(x) ((GRANDPA(x)->lchild == FATHER(x)) ? (GRANDPA(x)->rchild) : (GRANDPA(x)->lchild))

static void
LRotation(struct node *r)
{
	struct node *p = r->parent;
	struct node *c = r->lchild;
	r->lchild = c->rchild;
	if(c->rchild)
	{
		c->rchild->parent = r;
	}
	c->parent = p;
	c->rchild = r;
	r->parent = c;
	if(p)
	{
		if(p->lchild == r)
		{
			p->lchild = c;
		}
		else
		{
			p->rchild = c;
		}
	}
}

static void
RRotation(struct node *r)
{
	struct node *p = r->parent;
	struct node *c = r->rchild;
	r->rchild = c->lchild;
	if(c->lchild)
	{
		c->lchild->parent = r;
	}
	c->parent = p;
	c->lchild = r;
	r->parent = c;
	if(p)
	{
		if(p->rchild == r)
		{
			p->rchild = c;
		}
		else
		{
			p->lchild = c;
		}
	}
}

static void
rb_fix_up(struct node *r, struct node **root)
{
	while(COLOR(FATHER(r)) == RED)
	{
		if(COLOR(UNCLE(r)) == RED)
		{
			FATHER(r)->color = BLACK;
			UNCLE(r)->color = BLACK;
			GRANDPA(r)->color = RED;
			r = GRANDPA(r);
		}
		else if(GRANDPA(r)->lchild == FATHER(r))	// L
		{
			if(FATHER(r)->rchild == r) // R
			{
				r = FATHER(r);
				RRotation(r);
			}
			FATHER(r)->color = BLACK;
			GRANDPA(r)->color = RED;
			LRotation(GRANDPA(r));
			r = FATHER(r);
		}
		else	// R
		{
			if(FATHER(r)->lchild == r) // L
			{
				r = FATHER(r);
				LRotation(r);
			}
			FATHER(r)->color = BLACK;
			GRANDPA(r)->color = RED;
			RRotation(GRANDPA(r));
			r = FATHER(r);
		}
		if(FATHER(r) == NULL)
		{
			*root = r;
			return;
		}
	}
}

int 
insert(struct node **root, int v)
{
	struct node *p = NULL;
	struct node *n = (struct node *)malloc(sizeof(struct node));
	memset(n, '\0', sizeof(struct node));
	n->color = RED;
	n->v = v;
	struct node **tmp = root;
	for(; *tmp != NULL && (*tmp)->v != v && (p = *tmp); )
	{
		tmp = (*tmp)->v > v ? &(*tmp)->lchild : &(*tmp)->rchild;
	}
	if(*tmp && (*tmp)->v == v)
	{
		return 0;
	}
	n->parent = p;
	*tmp = n;
	rb_fix_up(n, root);
	(*root)->color = BLACK;
	return 1;
}

void display(struct node *root)
{
	if(root == NULL)
	{
		return ;
	}
	printf("%p:%d %s lchild(%p) rchild(%p)\n", root, root->v, root->color == BLACK ? "black" : "red", root->lchild, root->rchild);
	display(root->lchild);
	display(root->rchild);
}

static void
destroy(struct node *root)
{
	if(root == NULL)
	{
		return ;
	}
	destroy(root->lchild);
	destroy(root->rchild);
	free(root);
}

int 
get_max(struct node *root)
{
	if(root == NULL)
	{
		return INT_MAX;
	}
	while(root->rchild)
	{
		root = root->rchild;
	}
	return root->v;
}

int
get_min(struct node *root)
{
	if(root == NULL)
	{
		return INT_MAX;
	}
	while(root->lchild)
	{
		root = root->lchild;
	}
	return root->v;
}

int
main()
{
	int a[] = {12, 32, 19, 89, 84, 29, 39, 17, 10, 11, 3, 80, 79, 20, 54, 14, 78, 7, 123, 90};
#define N (sizeof(a) / sizeof(a[0]))

	struct node *root = NULL;
	for(int i = 0; i < N; ++i)
	{
		insert(&root, a[i]);
	}
	display(root);

	printf("min = %d max = %d\n", get_min(root), get_max(root));

	destroy(root);
	return 0;
}
