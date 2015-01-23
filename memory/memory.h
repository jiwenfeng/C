#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
union memory_block
{
	union memory_block *next;
	char data[1];
};

struct memory_bucket
{
	union memory_block *flist;
};

void memory_free(struct memory_bucket *bucket, void *p);
struct memory_bucket *memory_bucket_init();
void *memory_alloc(struct memory_bucket *bucket, size_t size);

#endif
