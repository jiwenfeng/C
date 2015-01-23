#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "../sniffer.h"
#include "../sniffer_log.h"

struct handler
{
	pthread_mutex_t lock;
	pthread_cond_t ready;
};

static void * _linux_thread_init(struct sniffer_base *base);
static void _linux_thread_cleanup(void *arg);
static int _linux_thread_create(struct sniffer_base *base, void *(*cb)(void *));

#define lock(handler)	pthread_mutex_lock(&handler->lock)
#define unlock(handler) pthread_mutex_unlock(&handler->lock)

#define cond_wait(handler, cond) while(cond){pthread_cond_wait(&handler->ready, &handler->lock);} 
#define cond_signal(handler) pthread_cond_signal(&handler->ready)

const struct sniffer_thread linux_thread =
{
	_linux_thread_init,
	_linux_thread_create,
	_linux_thread_cleanup
};


static void *
_linux_thread_init(struct sniffer_base *base)
{
	struct handler *h = (struct handler *)malloc(sizeof(struct handler));
	if(!h)
	{
		return NULL;
	}
	pthread_mutex_init(&h->lock, NULL);
	pthread_cond_init(&h->ready, NULL);
	return h;
}

static void
_linux_thread_cleanup(void *arg)
{
	if(!arg)
	{
		return;
	}
	struct handler *handler = (struct handler *)arg;
	pthread_mutex_destroy(&handler->lock);
	pthread_cond_destroy(&handler->ready);
	free(handler);
	handler = NULL;
}

static int
_linux_thread_create(struct sniffer_base *base, void *(*cb)(void *))
{
	pthread_t pid;
	int ret = pthread_create(&pid, NULL, cb, base);
	if(-1 == ret)
	{
		log(base, "%s", strerror(errno));
	}
	return ret;
}
