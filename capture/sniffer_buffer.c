#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "sniffer_buffer.h"
#include "sniffer_log.h"

#define LOCK(Q) while(__sync_lock_test_and_set(&Q->lock, 1)) {}
#define UNLOCK(Q) __sync_lock_release(&Q->lock)

struct addr
{
	ulong saddr;
	ulong daddr;
	ushort sport;
	ushort dport;
};

struct buffer
{
	int lock;
	int cap;
	int cur;
	char *data;
	struct buffer *next;
	struct buffer *prev;
	struct addr addr;
};

struct sniffer_buffer
{
	struct buffer *head;
	struct buffer *tail;
	struct buffer *iter;
	int lock;
};

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
	self->addr.saddr = sa;
	self->addr.daddr = da;
	self->addr.sport = sp;
	self->addr.dport = dp;
	self->data = (char *)malloc(self->cap);
	self->next = NULL;
	self->prev = NULL;
	return self;
}

void
buffer_destroy(struct buffer *b)
{
	free(b->data);
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
	if(sz > b->cur)
	{
		sz = b->cur;
	}
	b->cur -= sz;
	memmove(b->data, b->data + sz, b->cur);
	UNLOCK(b);
}

char *
buffer_peek(struct buffer *b, int *sz)
{
	LOCK(b);
	if(b->cur == 0)
	{
		UNLOCK(b);
		return NULL;
	}
	int len = sizeof(struct addr);
	char *data = (char *)malloc(b->cur + len);
	memcpy(data, &b->addr, len);
	memcpy(data + len, b->data, b->cur);
	*sz = b->cur;
	UNLOCK(b);
	return data + len;
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
	self->tail = NULL;
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
		buffer_destroy(b);
		iter = iter->next;
	}
	free(self);
}

int
sniffer_buffer_push(struct sniffer_buffer *self, ulong sa, ulong da, ushort sp, ushort dp, const char *data, int sz)
{
	struct buffer *itr = self->head;
	while(itr != NULL)
	{
		struct addr *addr = &itr->addr;
		if(addr->saddr == sa && addr->daddr == da && addr->sport == sp && addr->dport == dp)
		{
			return buffer_push(itr, data, sz);
		}
		itr = itr->next;
	}
	itr = buffer_init(sa, da, sp, dp);
	if(NULL == itr)
	{
		return 0;
	}
	if(NULL == self->head)
	{
		self->head = itr;
		self->tail = itr;
	}
	else
	{
		itr->prev = self->tail;
		self->tail->next = itr;
		self->tail = itr;
	}
	return buffer_push(itr, data, sz);
}

char *
sniffer_buffer_peek(struct sniffer_buffer *self, int *sz)
{
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
			return str;
		}
		if(self->iter == b)
		{
			break;
		}
	}
	return NULL;
}

void
sniffer_buffer_remove(struct sniffer_buffer *self, char *str, int sz)
{
	struct addr *addr = (struct addr *)(str - sizeof(struct addr));
	struct buffer *b = self->head;
	while(b != NULL)
	{
		if(b->addr.saddr == addr->saddr && b->addr.daddr == addr->daddr && b->addr.sport == addr->sport && b->addr.dport == addr->dport)
		{
			buffer_remove(b, sz);
			free(addr);
			return ;
		}
		b = b->next;
	}
}
