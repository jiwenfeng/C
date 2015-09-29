#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct node
{
	int k;
	int v;
	struct node *l;
	struct node *r;
	struct node *next;
};

void
list_insert(struct node **head, int k, int v)
{
	struct node *node = (struct node *)malloc(sizeof(struct node));
	node->k = k;
	node->v = v;
	node->l = node->r = node->next = NULL;
	for(; *head != NULL; head = &(*head)->next)
	{
		if((*head)->k > k)
		{
			node->next = *head;
			break;
		}
	}
	*head = node;
}

void
list_push(struct node **head, struct node *node)
{
	for(; *head != NULL; head = &(*head)->next)
	{
		if((*head)->k > node->k)
		{
			node->next = *head;
			break;
		}
	}
	*head = node;
}

void
list_pop(struct node **head, struct node *node)
{
	for(; *head != NULL; head = &(*head)->next)
	{
		if((*head)->k == node->k)
		{
			*head = node->next;
			node->next = NULL;
			break;
		}
	}
}

void
list_delete(struct node **head, int k)
{
	for(; *head != NULL; head = &(*head)->next)
	{
		if((*head)->k == k)
		{
			struct node *node = *head;
			*head = node->next;
			free(node);
			break;
		}
	}
}

void
list_debug(struct node *head)
{
	while(head)
	{
		printf("%d ", head->k);
		head = head->next;
	}
	printf("\n");
}

void
list_destroy(struct node *head)
{
	while(head)
	{
		struct node *node = head;
		head = head->next;
		free(node);
	}
}

void
tree_destroy(struct node *root)
{
	if(NULL == root)
	{
		return ;
	}
	tree_destroy(root->l);
	tree_destroy(root->r);
	free(root);
}

void
build_huffman_tree(struct node **head, struct node **root)
{
	while(1)
	{
		struct node *node = (*head)->next;
		if(NULL == node)
		{
			break;
		}
		*root = (struct node *)malloc(sizeof(struct node));
		(*root)->k = (*head)->k + node->k;
		(*root)->l = *head;
		(*root)->r = node;
		list_pop(head, *head);
		list_pop(head, node);
		list_push(head, *root);
	}
}

void
tree_debug(struct node *root)
{
	if(NULL == root)
	{
		return ;
	}
	tree_debug(root->l);
	printf("%d ", root->k);
	tree_debug(root->r);
}

int
main()
{
	int a[] = {32, 12, 43, 89, 21, 8, 10, 34, 11};
#define N sizeof(a) / sizeof(a[0])
	int i = 0;
	struct node *head = NULL;
	for(i = 0; i < N; ++i)
	{
		list_insert(&head, a[i], a[i]);
	}
	struct node *root = NULL;
	list_debug(head);
	build_huffman_tree(&head, &root);
	tree_debug(root);
	printf("\n");
	tree_destroy(root);
	return 0;
}
