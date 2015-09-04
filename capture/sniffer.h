#ifndef __SNIFFER_H__
#define __SNIFFER_H__

#include <stdint.h>
#include <pthread.h>

struct sniffer_buffer;
struct sniffer_capture;
struct sniffer_module;

struct sniffer_config
{
	uint16_t	port;
	uint32_t	s_addr;
	uint32_t	d_addr;
	char		*proto;
	char		*decode;
	void		*stream;
};

struct sniffer_base
{
	struct sniffer_capture *cap;
	struct sniffer_buffer *sb;
	struct sniffer_module *sm;
	struct sniffer_config *cfg;
	int term;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

struct sniffer_base *sniffer_init(const char *path);
void sniffer_start(struct sniffer_base *sb);
void sniffer_destroy(struct sniffer_base *sb);

#endif
