#ifndef __SNIFFER_H__
#define __SNIFFER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sniffer_buffer;
struct sniffer_op;
struct sniffer_thread;

struct sniffer_cfg
{
	unsigned short port;
};

struct sniffer_base
{
	int status;
	struct sniffer_cfg *cfg;
	struct sniffer_buffer *sb_send;
	struct sniffer_buffer *sb_recv;
	struct sniffer_buffer *sb_cur;
	struct sniffer_buffer *sb_output;
	const struct sniffer_op *op;
	const struct sniffer_thread *thread;
	void *thread_handler;
	void *op_handler;
	void *parse_handler;
	FILE *log_handler;
};

struct sniffer_op
{
	void *(*init)(struct sniffer_base *base);
	int(*dispatch)(struct sniffer_base *base);
	void(*cleanup)(void *handler);
};

struct sniffer_thread
{
	void *(*init)(struct sniffer_base *base);
	int (*create)(struct sniffer_base *base, void *(*)(void *));
	void (*cleanup)(void *handler);
};

struct sniffer_base *sniffer_init();
void sniffer_suspend(struct sniffer_base *base);
void sniffer_resume(struct sniffer_base *base);
void sniffer_shutdown(struct sniffer_base *base);
int sniffer_loop(struct sniffer_base *base);

#endif
