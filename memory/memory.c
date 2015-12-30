#include <memory.h>
#include <string.h>
#include <stdlib.h>

#define NBLOCKS (1024 / 8)

#define BLOCK 1024 * 1000

static int _get_n(int n);
static void mm_alloc(struct memory_bucket *bucket, int n);

static void
mm_alloc(struct memory_bucket *bucket, int pos)
{
	void *p = malloc(BLOCK);
	int real = (pos + 1) * 8;
	int n = BLOCK / real;
	char *head = (char *)p;
	while(n-- > 1)
	{
		char *b = (char *)head + real;
		((union memory_block *)head)->next = (union memory_block *)b;
		head = b;
		((union memory_block *)b)->next = NULL;
	}
	bucket->flist[pos].next = (union memory_block *)p;
}

static int
_get_n(int n)
{
	return ((n + 7) / 8 - 1);
}

void *
memory_alloc(struct memory_bucket *bucket, size_t size)
{
	size_t real = size + sizeof(size_t);
	int n = _get_n(real);
	if(n > NBLOCKS)
	{
		void *p = malloc(real);
		memset(p, real, real);
		return p + sizeof(size_t);
	}
	else
	{
		if(!bucket->flist[n].next)
		{
			mm_alloc(bucket, n);
		}
		if(!bucket->flist[n].next)
		{
			return NULL;
		}
		union memory_block **head = &bucket->flist[n].next;
		char *p = (char *)*head;
		*head = (*head)->next;
		memset(p, real, real);
		return p + sizeof(size_t);
	}
	return NULL;
}

void 
memory_free(struct memory_bucket *bucket, void *p)
{
	char *realptr = (char *)p - sizeof(size_t);
	int size = *realptr;
	int n = _get_n(size);
	if(n > NBLOCKS)
	{
		free(realptr);
	}
	else
	{
		((union memory_block *)realptr)->next = bucket->flist[n].next;
		bucket->flist[n].next = (union memory_block *)realptr;
	}
}

struct memory_bucket *
memory_bucket_init()
{
	struct memory_bucket *bucket = (struct memory_bucket *)malloc(sizeof(struct memory_bucket));
	bucket->flist = (union memory_block *)malloc(sizeof(union memory_block) * NBLOCKS);
	return bucket;
}

void 
memory_bucket_destroy(struct memory_bucket *bucket)
{
	int i = 0;
	for(i = 0; i < NBLOCKS; ++i)
	{
		if(bucket->flist[i].next != NULL)
		{
			free(bucket->flist[i].next);
		}
	}
	free(bucket->flist);
	free(bucket);
}

int 
main()
{
	struct memory_bucket *bucket = memory_bucket_init();

	const char *str = "hello world";
	size_t n = strlen(str) + 1;
	char *p = (char *)memory_alloc(bucket, n);

	memset(p, '\0', n);

	memcpy(p, str, n - 1);

	printf("%s\n", p);

	memory_free(bucket, p);

	memory_bucket_destroy(bucket);
	return 0;
}
