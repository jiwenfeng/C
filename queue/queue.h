#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define QUEUE_ENTRY(type, name)	\
struct name{	\
	struct type *tqe_next;	\
	struct type **tqe_prev;	\
}

#define QUEUE_HEAD(type)	\
struct {	\
	struct type *tqh_first;	\
	struct type **tqh_last;	\
}

#define QUEUE_INIT(head)	do{	\
	(head)->tqh_first = NULL;	\
	(head)->tqh_last = &(head)->tqh_first;	\
}while(0)

#define QUEUE_FIRST(head)	((head)->tqh_first)
#define QUEUE_END(head)		NULL
#define QUEUE_NEXT(var, field)	(var)->field.tqe_next

#define QUEUE_FOREACH(var, head, field)	\
	for((var) = QUEUE_FIRST(head); (var) != QUEUE_END(head); (var) = QUEUE_NEXT(var, field))

#define QUEUE_INSERT_HEAD(head, var, field)	do{	\
	if(((var)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(var)->field.tqe_prev = &(head)->tqh_first;	\
	else	\
		*(head)->tqh_last = (var);	\
	(head)->tqh_first = (var);	\
}while(0)

#define QUEUE_INSERT_TAIL(head, var, field) do{	\
	(var)->field.tqe_next = NULL;	\
	(var)->field.tqe_prev = (head)->tqh_last;	\
	*(head)->tqh_last = (var);	\
	(head)->tqh_last = &(var)->field.tqe_next;	\
}while(0)

#define QUEUE_REMOVE(head, var, field)	do{	\
\
}while(0)
