#include <queue.h>

struct buffer
{
	QUEUE_ENTRY(buffer, ptr) next;
	size_t len;
	char data[1];
};

struct buffer_chain
{
	QUEUE_HEAD(buffer) list;
	size_t total;
};

struct buffer_chain *
init_buffer_chain()
{
	struct buffer_chain *bc = (struct buffer_chain *)malloc(sizeof(struct buffer_chain));
	if(!bc)
	{
		return NULL;
	}
	bc->total = 0;
	QUEUE_INIT(&bc->list);
	return bc;
}

static void 
buffer_add_data(struct buffer_chain *bc, const char *data, size_t datalen)
{
	size_t len = datalen + sizeof(struct buffer);
	struct buffer *b = (struct buffer *)malloc(len);
	if(!b)
	{
		return;
	}
	b->len = datalen;
	memcpy(b->data, data, datalen);
	QUEUE_INSERT_HEAD(&bc->list, b, next);
}

int 
main()
{
	struct buffer_chain *bc = init_buffer_chain();
	if(!bc)
	{
		return 1;
	}
	const char *str[] = {
		"helloworld1", "helloworld2", "helloworld3", "helloworld4", "helloworld5", 
		"helloworld6", "helloworld7", "helloworld8", "helloworld9", "helloworld0"
	};
	int num = sizeof(str) / sizeof(str[0]), i = 0;
	for(i = 0; i < num; i++)
	{
		buffer_add_data(bc, str[i], strlen(str[i]));
	}
	struct buffer *b;
	QUEUE_FOREACH(b, &bc->list, next)
	{
		printf("%s\n", b->data);
	}
	return 0;
}

