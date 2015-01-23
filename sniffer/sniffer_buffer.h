#ifndef __SNIFFER_MQ__
#define __SNIFFER_MQ__

struct sniffer_buffer
{
	int cur;	/* 字符长度 */
	int total;	/* 缓冲区长度 */
	char *data;
};

struct sniffer_buffer * sniffer_buffer_init();
void sniffer_buffer_push(struct sniffer_buffer *buffer, const char *data, int ss);
const char *sniffer_buffer_peekup(struct sniffer_buffer *buffer);
char *sniffer_buffer_pop(struct sniffer_buffer *buffer);
int sniffer_buffer_remove(struct sniffer_buffer *buffer, int ss);
void sniffer_buffer_cleanup(struct sniffer_buffer *buffer);
int sniffer_buffer_size(struct sniffer_buffer *buffer);

#endif
