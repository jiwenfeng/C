#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define LEVEL 10

struct skip_node
{
	int v;
	int level;
	struct skip_node *forward[1];
};

struct skip_list
{
	int sz;
	int level;
	struct skip_node *head;
};

struct skip_node *
skip_node_alloc(int level, int v)
{
	size_t sz = sizeof(struct skip_node) + sizeof(struct skip_node *) * level;
	struct skip_node *node = (struct skip_node *)malloc(sz);
	node->v = v;
	memset(node->forward, '\0', sizeof(struct node *) * level);
	return node;
}

struct skip_list *
skip_list_init()
{
	struct skip_list *self = (struct skip_list *)malloc(sizeof(struct skip_list));
	self->head = skip_node_alloc(LEVEL, 0);		// dummy head
	self->sz = 0;
	return self;
}

void
skip_list_insert(struct skip_list *self, int n)
{
	struct skip_node *update[LEVEL] = {0};
	struct skip_node *head = self->head;
	int i;
	for(i = LEVEL - 1 ; i >= 0; --i)
	{
		while(head->forward[i] && head->forward[i]->v > n)
		{
			head = head->forward[i];
		}
		update[i] = head;
	}

	if(head->v == n) // find equal return
	{
		return ;
	}

	int level = rand() % LEVEL;
	if(level == 0)
	{
		level = 1;
	}

	struct skip_node *node = skip_node_alloc(level, n);
	node->level = level;

	for(i = level - 1; i >= 0; --i)
	{
		node->forward[i] = update[i]->forward[i];
		update[i]->forward[i] = node;
	}
	self->sz++;
}

void
skip_list_delete(struct skip_list *self, int n)
{
	struct skip_node *update[LEVEL] = {0};
	struct skip_node *head = self->head;
	int i;
	for(i = LEVEL - 1 ; i >= 0; --i)
	{
		while(head->forward[i] && head->forward[i]->v > n)
		{
			head = head->forward[i];
		}
		update[i] = head;
	}

	head = head->forward[0];

	if(head == NULL || head->v != n) 
	{
		return ;
	}

	for(i = 0; i < head->level; ++i)
	{
		update[i]->forward[i] = head->forward[i];
	}

	free(head);

	--self->sz;

}

void
skip_list_debug(struct skip_list *self)
{
	struct skip_node *head = self->head->forward[0];
	while(head)
	{
		printf("%p = %d\n", head, head->v);
		int i = 0;
		for(i = 0; i < head->level; ++i)
		{
			printf("%d = %p\n", i, head->forward[i]);
		}
		head = head->forward[0];
	}
	printf("\n");
}

int
skip_list_search(struct skip_list *self, int n)
{
	struct skip_node *head = self->head;
	int i = 0;
	for(i = LEVEL - 1; i >= 0; --i)
	{
		while(head->forward[i] && head->forward[i]->v >= n)
		{
			if(head->forward[i]->v == n)
			{
				return 1;
			}
			head = head->forward[i];
		}
	}
	return 0;
}

void
skip_list_destroy(struct skip_list *self)
{
	struct skip_node *head = self->head;
	while(head)
	{
		struct skip_node *h = head;
		head = head->forward[0];
		free(h);
	}
	free(self);
}

int 
main()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	int a[] = {32, 12, 4, 31, 34, 51, 90, 87, 6, 42};
#define N sizeof(a) / sizeof(a[0])
	
	int i;

	for(i = 0; i< N; ++i)
	{
		printf("%d ", a[i]);
	}
	printf("\n");

	struct skip_list *skip = skip_list_init();

	for(i = 0; i < N; ++i)
	{
		skip_list_insert(skip, a[i]);
	}
	
	skip_list_debug(skip);

	skip_list_delete(skip, 34);

	skip_list_debug(skip);

	printf("%d %s and %d %s\n", 42, skip_list_search(skip, 42) == 1 ? "exist" : "not exist", 0, skip_list_search(skip, 0) == 1 ? "exist" : "not exist");

	skip_list_destroy(skip);

	return 0;
}
