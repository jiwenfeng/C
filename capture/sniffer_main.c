#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "sniffer.h"
#include "sniffer_log.h"
#include "sniffer_buffer.h"
#include "sniffer_config.h"
#include "sniffer_capture.h"
#include "sniffer_module.h"

#define MAX_PACKET_LEN 1024 * 50

static struct sniffer_base *global_base = NULL;

static struct sniffer_base *sniffer_base_init(const char *path);
static void sniffer_base_destroy(struct sniffer_base *sb);
static void *capture(void *arg);
static void *decode(void *arg);
static void proc_sig(int sig);

struct sniffer_base *
sniffer_base_init(const char *path)
{
	struct sniffer_base *base = (struct sniffer_base *)malloc(sizeof(struct sniffer_base) + sizeof(pthread_t) * 2);
	if(NULL == base)
	{
		return NULL;
	}
	base->thread_num = 2;
	memset(base, '\0', sizeof(struct sniffer_base));
	base->cfg = sniffer_config_init(path);
	if(NULL == base->cfg)
	{
		sniffer_base_destroy(base);
		return NULL;
	}
	base->cap = sniffer_capture_init();
	if(NULL == base->cap)
	{
		sniffer_base_destroy(base);
		return NULL;
	}
	base->sb = sniffer_buffer_init();
	if(NULL == base->sb)
	{
		sniffer_base_destroy(base);
		return NULL;
	}
	base->sm = sniffer_module_init(base->cfg->decode);
	if(NULL == base->sm)
	{
		sniffer_base_destroy(base);
		return NULL;
	}
	base->term = 0;
	return base;
}

void 
sniffer_base_destroy(struct sniffer_base *base)
{
	if(base->cap != NULL)
	{
		sniffer_capture_destroy(base->cap);
	}
	if(base->sb != NULL)
	{
		sniffer_buffer_destroy(base->sb);
	}
	if(base->sm != NULL)
	{
		sniffer_module_destroy(base->sm);
	}
	if(base->cfg != NULL)
	{
		sniffer_config_destroy(base->cfg);
	}
	pthread_mutex_destroy(&base->mutex);
	pthread_cond_destroy(&base->cond);
	free(base);
}

struct sniffer_base *
sniffer_init(const char *path)
{
	struct sniffer_base *base = sniffer_base_init(path);
	if(NULL == base)
	{
		return NULL;
	}
	pthread_mutex_init(&base->mutex, NULL);
	pthread_cond_init(&base->cond, NULL);
	global_base = base;
	signal(SIGINT, proc_sig);
	return base;
}

void *
capture(void *arg)
{
	struct sniffer_base *base = (struct sniffer_base *)arg;
	while(1)
	{
		if(1 == base->term)
		{
			pthread_cond_broadcast(&base->cond);
			break;
		}
		int n = sniffer_capture_cap(base);
		if(n > 0)
		{
			pthread_cond_signal(&base->cond);
			continue;
		}
		if(n < 0)
		{
			base->term = 1;
		}
	}
	return NULL;
}

static void 
cleanup_decode_thread(void *arg)
{
	struct sniffer_base *base = (struct sniffer_base *)arg;
	pthread_mutex_unlock(&base->mutex);
}

void *
decode(void *arg)
{
	struct sniffer_base *base = (struct sniffer_base *)arg;
	pthread_cleanup_push(cleanup_decode_thread, base);
	while(1)
	{
		if(1 == base->term)
		{
			return NULL;
		}
		int sz = 0;
		char *str = sniffer_buffer_peek(base->sb, &sz);
		if(NULL == str)
		{
			pthread_mutex_lock(&base->mutex);
			pthread_cond_wait(&base->cond, &base->mutex);
			pthread_mutex_unlock(&base->mutex);
			continue;
		}
		char packet[MAX_PACKET_LEN] = {0};
		int len = sniffer_module_callback(base->sm, str, sz, packet, MAX_PACKET_LEN); 
		sniffer_buffer_remove(base->sb, str, len);
		if(len == 0)
		{
			continue;
		}
		sniffer_report(base->cfg->stream, "%s", packet);
	}
	pthread_cleanup_pop(1);
	return NULL;
}

void
proc_sig(int sig)
{
	if(SIGINT == sig)
	{
		int i = 0;
		for(i = 0; i < global_base->thread_num; ++i)
		{
			pthread_cancel(global_base->thread_id[i]);
		}
	}
}

void 
sniffer_start(struct sniffer_base *base)
{
	pthread_create(&base->thread_id[0], NULL, capture, base);
	pthread_create(&base->thread_id[1], NULL, decode, base);
	int i = 0;
	for(i = 0; i < 2; ++i)
	{
		pthread_join(base->thread_id[i], NULL);
	}
}

void 
sniffer_destroy(struct sniffer_base *base)
{
	sniffer_base_destroy(base);
	global_base = NULL;
}

int
main(int argc, char *argv[])
{
	struct sniffer_base *base = sniffer_init("./config");
	if(NULL == base)
	{
		return 0;
	}
	sniffer_start(base);
	sniffer_destroy(base);
	return 0;
}
