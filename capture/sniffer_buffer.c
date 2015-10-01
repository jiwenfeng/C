#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "sniffer_buffer.h"
#include "sniffer_log.h"

#define LOCK(Q) while(__sync_lock_test_and_set(&Q->lock, 1)) {}
#define UNLOCK(Q) __sync_lock_release(&Q->lock)

struct session
{
	ulong sa;
	ulong da;
	ushort sp;
	ushort dp;
};

struct buffer
{
	int lock;
	int cap;
	int cur;
	int offset;
	char *data;
	struct buffer *next;
	struct session *s;
};

struct sniffer_buffer
{
	struct buffer *head;
	struct buffer *iter;
	int lock;
};

#define IS_SAME_SESSION(a1, a2) ((a1)->sa == (a2)->sa && (a1)->da == (a2)->da && (a1)->sp == (a2)->sp && (a1)->dp == (a2)->dp)

static struct buffer *buffer_init(ulong sa, ulong da, ushort sp, ushort dp);
static void buffer_destroy(struct buffer *b);
static int buffer_push(struct buffer *b, const char *data, int sz);
static char *buffer_peek(struct buffer *b, int *sz);

struct buffer *
buffer_init(ulong sa, ulong da, ushort sp, ushort dp)
{
	struct buffer *self = (struct buffer *)malloc(sizeof(struct buffer));
	self->lock = 0;
	self->cap  = 65535;
	self->cur  = 0;
	self->offset = 0;
	struct session *s= (struct session *)malloc(sizeof(struct session));
	s->sa = sa;
	s->da = da;
	s->sp = sp;
	s->dp = dp;
	self->s = s;
	self->data = (char *)malloc(self->cap);
	self->next = NULL;
	return self;
}

void
buffer_destroy(struct buffer *b)
{
	free(b->data);
	free(b->s);
	free(b);
}

int
buffer_push(struct buffer *b, const char *data, int sz)
{
	LOCK(b);
	if(b->cur + sz > b->cap)
	{
		int new_sz = (b->cap + sz) * 2;
		b->data = realloc(b->data, new_sz);
		b->cap = new_sz;
	}
	memmove(b->data + b->cur, data, sz);
	b->cur += sz;
	UNLOCK(b);
	return sz;
}

void 
buffer_remove(struct buffer *b, int sz)
{
	LOCK(b);
	if(sz > b->offset)
	{
		sz = b->offset;
	}
	b->offset -= sz;
	b->cur -= sz;
	memmove(b->data, b->data + sz, b->offset);
	UNLOCK(b);
}

char *
buffer_peek(struct buffer *b, int *sz)
{
	LOCK(b);
	if(b->offset >= b->cur)
	{
		UNLOCK(b);
		return NULL;
	}
	*sz = b->cur - b->offset;
	char *data = (char *)malloc(sizeof(struct session) + *sz);
	memcpy(data, b->s, sizeof(struct session));
	memcpy(data + sizeof(struct session), b->data + b->offset, *sz);
	b->offset = b->cur;
	UNLOCK(b);
	return data + sizeof(struct session);
}

struct sniffer_buffer *
sniffer_buffer_init()
{
	struct sniffer_buffer *self = (struct sniffer_buffer *)malloc(sizeof(struct sniffer_buffer));
	if(NULL == self)
	{
		return NULL;
	}
	self->head = NULL;
	self->iter = NULL;
	self->lock = 0;
	return self;
}

void
sniffer_buffer_destroy(struct sniffer_buffer *self)
{
	struct buffer *iter = self->head;
	while(iter != NULL)
	{
		struct buffer *b = iter;
		iter = iter->next;
		buffer_destroy(b);
	}
	free(self);
}

int
sniffer_buffer_push(struct sniffer_buffer *self, ulong sa, ulong da, ushort sp, ushort dp, const char *data, int sz)
{
	LOCK(self);
	struct buffer **itr = NULL;
	for(itr = &self->head; *itr != NULL; itr = &(*itr)->next)
	{
		struct session *s = (*itr)->s;
		if(s->sa == sa && s->da == da && s->sp == sp && s->dp == dp)
		{
			UNLOCK(self);
			return buffer_push(*itr, data, sz);
		}
	}
	*itr = buffer_init(sa, da, sp, dp);
	UNLOCK(self);
	return buffer_push(*itr, data, sz);
}

char *
sniffer_buffer_peek(struct sniffer_buffer *self, int *sz)
{
	LOCK(self);
	struct buffer *b = self->iter;
	while(self->head != NULL)
	{
		if(NULL == self->iter)
		{
			self->iter = self->head;
		}
		char *str = buffer_peek(self->iter, sz);
		self->iter = self->iter->next;
		if(str != NULL)
		{
			UNLOCK(self);
			return str;
		}
		if(self->iter == b)
		{
			break;
		}
	}
	UNLOCK(self);
	return NULL;
}

void
sniffer_buffer_remove(struct sniffer_buffer *self, char *str, int sz)
{
	LOCK(self);
	struct session *s = (struct session *)(str - sizeof(struct session));
	struct buffer *b = self->head;
	while(b)
	{
		if(IS_SAME_SESSION(b->s, s))
		{
			buffer_remove(b, sz);
			break;
		}
		b = b->next;
	}
	free(s);
	UNLOCK(self);
}

int sniffer_buffer_delete(struct sniffer_buffer *self, ulong sa, ulong da, ushort sp, ushort dp)
{
	LOCK(self);
	struct buffer **b = NULL;
	for(b = &self->head; *b != NULL; b = &(*b)->next)
	{
		struct session *s = (*b)->s;
		if(s->sa == sa && s->da == da && s->sp == sp && s->dp == dp)
		{
			struct buffer *t = *b;
			*b = (*b)->next;
			buffer_destroy(t);
			break;
		}
	}
	UNLOCK(self);
	return 0;
}
