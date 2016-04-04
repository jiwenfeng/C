#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <unistd.h>

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

#define COLOR(x) ((x) ? ((x)->color) : BLACK)

#define GRANDPA(x) FATHER(FATHER(x))

#define FATHER(x) ((x)->parent)

#define UNCLE(x) ((GRANDPA(x)->lchild == FATHER(x)) ? (GRANDPA(x)->rchild) : (GRANDPA(x)->lchild))

#define SWAP(i1, i2) do {int val = (i1)->v; (i1)->v = (i2)->v; (i2)->v = val; } while (0)

static void
rb_r_rotation(struct node *r)
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
rb_l_rotation(struct node *r)
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
rb_insert_fixup(struct node *r, struct node **root)
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
				rb_l_rotation(r);
			}
			FATHER(r)->color = BLACK;
			GRANDPA(r)->color = RED;
			rb_r_rotation(GRANDPA(r));
			r = FATHER(r);
		}
		else	// R
		{
			if(FATHER(r)->lchild == r) // L
			{
				r = FATHER(r);
				rb_r_rotation(r);
			}
			FATHER(r)->color = BLACK;
			GRANDPA(r)->color = RED;
			rb_l_rotation(GRANDPA(r));
			r = FATHER(r);
		}
		if(NULL == FATHER(r))
		{
			*root = r;
			break;
		}
	}
	(*root)->color = BLACK;
}

int 
insert(struct node **root, int v)
{
	struct node **itr = root;
	struct node *p = NULL;
	while(*itr != NULL)
	{
		if((*itr)->v == v)
		{
			return 0;
		}
		p = *itr;
		itr = (*itr)->v > v ? &(*itr)->lchild : &(*itr)->rchild;
	}
	struct node *n = (struct node *)malloc(sizeof(struct node));
	n->lchild = n->rchild = NULL;
	n->parent = p;
	n->v = v;
	n->color = RED;
	*itr = n;
	rb_insert_fixup(n, root);
	return 1;
}

static void
rb_delete_fixup(struct node **root, struct node *p, struct node *n)
{
	while(COLOR(n) == BLACK && n != *root)
	{
		if(p->lchild == n)
		{
			struct node *b = p->rchild;
			if(COLOR(b) == RED)
			{
				b->color = BLACK;
				p->color = RED;
				rb_r_rotation(p);
				b = p->rchild;
			}
			if(COLOR(b->lchild) == BLACK && COLOR(b->rchild) == BLACK)
			{
				b->color = RED;
				n = p;
				p = FATHER(p);
			}
			else
			{
				if(COLOR(b->lchild) == RED)
				{
					b->lchild->color = BLACK;
					b->color = RED;
					rb_r_rotation(b);
					b = p->rchild;
				}
				b->color = p->color;
				p->color = BLACK;
				b->rchild->color = BLACK;
				n = *root;
				rb_l_rotation(p);
			}
		}
		else
		{
			struct node *b = p->lchild;
			if(COLOR(b) == RED)
			{
				b->color = BLACK;
				p->color = RED;
				rb_l_rotation(p);
				b = p->lchild;
			}
			if(COLOR(b->lchild) == BLACK && COLOR(b->rchild) == BLACK)
			{
				b->color = RED;
				n = p;
				p = FATHER(p);
			}
			else
			{
				if(COLOR(b->rchild) == RED)
				{
					b->lchild->color = BLACK;
					b->color = RED;
					rb_l_rotation(b);
					b = p->lchild;
				}
				b->color = p->color;
				p->color = BLACK;
				b->lchild->color = BLACK;
				n = *root;
				rb_r_rotation(p);
			}
		}

	}
	if(n != NULL)
	{
		n->color = BLACK;
	}
}

static void
rb_delete(struct node **root, struct node *n)
{
	struct node **itr = &n->rchild;
	while(*itr != NULL)
	{
		if((*itr)->lchild == NULL)
		{
			break;
		}
		itr = &(*itr)->lchild;
	}
	struct node *x = *itr;
	if(x != NULL)
	{
		if(x->rchild != NULL)
		{
			FATHER(x->rchild) = FATHER(x);
		}
		*itr = x->rchild;
		SWAP(n, x);
	}
	else
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
	if(COLOR(x) == BLACK)
	{
		rb_delete_fixup(root, FATHER(x), x->rchild);
	}
	free(x);
}

void 
delete(struct node **root, int n)
{
	struct node *itr = *root;
	for(; itr && itr->v != n; itr = itr->v > n ? itr->lchild : itr->rchild)
	{
//		itr = itr->v > n ? itr->lchild : itr->rchild;
	}
	if(NULL == itr)
	{
		return;
	}
	rb_delete(root, itr);
}

static void 
debug(struct node *root)
{
	if(NULL == root)
	{
		return ;
	}
	printf("%p:%d %s lchild(%p) rchild(%p)\n", root, root->v, root->color == BLACK ? "black" : "red", root->lchild, root->rchild);
	debug(root->lchild);
	debug(root->rchild);
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
	int a[] = {12, 32, 11, 23, 90, 84, 75, 81, 30, 20, 10, 76, 1, 3, 98, 72, 25, 80};
	int i;
#define N (sizeof(a) / sizeof(a[0]))

	struct node *root = NULL;
	for(i = 0; i < N; ++i)
	{
		insert(&root, a[i]);
	}
	debug(root);
	printf("min = %d max = %d\n", get_min(root), get_max(root));
	delete(&root, 32);
	debug(root);
	destroy(root);
	return 0;
}
