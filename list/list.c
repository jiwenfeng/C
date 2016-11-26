#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct node 
{
	int v;
	struct node *next;
};

void insert(struct node **head, int v)
{
	struct node *n = (struct node *)malloc(sizeof(struct node));
	n->v = v;
	n->next = NULL;
	for(; *head != NULL; head = &(*head)->next)
	{
		if((*head)->v > v)
		{
			n->next = *head;
			break;
		}
	}
	*head = n;
}

void delete(struct node **head, int v)
{
	for(; *head != NULL; head = &(*head)->next)
	{
		if((*head)->v == v)
		{
			struct node *tmp = *head;
			*head = tmp->next;
			free(tmp);
			break;
		}
	}
}

void destroy(struct node *head)
{
	while(head)
	{
		struct node *n = head;
		head = head->next;
		free(n);
	}
}

static void
display(struct node *head)
{
	for(; head != NULL; head = head->next)
	{
		printf("%d ", head->v);
	}
	printf("\n");
}

int 
main()
{
	int a[] = {32, 12, 34, 31, 39, 2, 1, 0, 5};
#define N (sizeof(a) / sizeof(a[0]))
	int i;
	struct node *head = NULL;
	for(i = 0; i < N; ++i)
	{
		insert(&head, a[i]);
	}
	display(head);

	delete(&head, 32);
	display(head);

	delete(&head, 0);
	display(head);

	delete(&head, 39);
	display(head);


	destroy(head);
	return 0;
}

