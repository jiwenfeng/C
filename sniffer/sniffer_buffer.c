#include "sniffer_buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_BUFFER_LEN 1024

static void _sniffer_buffer_expand(struct sniffer_buffer *buffer, int ss);

static void 
_sniffer_buffer_expand(struct sniffer_buffer *buffer, int ss)
{
	char *new_buffer = (char *)malloc(ss);
	memset(new_buffer, '\0', ss);
	memcpy(new_buffer, buffer->data, buffer->cur);
	buffer->total = ss;
	free(buffer->data);
	buffer->data = new_buffer;
}

struct sniffer_buffer *
sniffer_buffer_init()
{
	struct sniffer_buffer *buffer = (struct sniffer_buffer *)malloc(sizeof(struct sniffer_buffer));
	if(!buffer)
	{
		return NULL;
	}
	memset(buffer, '\0', sizeof(struct sniffer_buffer));
	buffer->cur = 0;
	buffer->total = DEFAULT_BUFFER_LEN;
	buffer->data = (char *)malloc(DEFAULT_BUFFER_LEN);
	return buffer;
}

void 
sniffer_buffer_push(struct sniffer_buffer *buffer, const char *data, int ss)
{
	if(buffer->cur + ss >= buffer->total)
	{
		_sniffer_buffer_expand(buffer, buffer->cur + ss);
	}
	memcpy(buffer->data + buffer->cur, data, ss);
	buffer->cur += ss;
}

char *
sniffer_buffer_pop(struct sniffer_buffer *buffer)
{
	char *buf = (char *)malloc(buffer->cur + 1);
	memset(buf, '\0', buffer->cur + 1);
	memcpy(buf, buffer->data, buffer->cur);
	buffer->cur = 0;
	memset(buffer->data, '\0', buffer->cur);
	return buf;
}

const char *
sniffer_buffer_peekup(struct sniffer_buffer *buffer)
{
	return buffer->data;
}

int
sniffer_buffer_remove(struct sniffer_buffer *buffer, int ss)
{
	if(ss <= 0 || ss >= buffer->cur)
	{
		buffer->cur = 0;
		memset(buffer->data, '\0', buffer->cur);
		return 0;
	}
	buffer->cur -= ss;
	memmove(buffer->data, buffer->data + ss, buffer->cur);
	memset(buffer->data + buffer->cur, '\0', buffer->total - buffer->cur);
	return 0;
}

int
sniffer_buffer_size(struct sniffer_buffer *buffer)
{
	if(!buffer)
	{
		return 0;
	}
	return buffer->cur;
}

void 
sniffer_buffer_cleanup(struct sniffer_buffer *buffer)
{
	if(buffer)
	{
		if(buffer->data)
		{
			free(buffer->data);
			buffer->data = NULL;
		}
		free(buffer);
		buffer = NULL;
	}
}
