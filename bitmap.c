#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>

#define WORD_WIDE (sizeof(int) * 8)

#define BIT_SET(map, n) map[(n >> 5)] |= (1 << (n & (WORD_WIDE - 1)))

#define IS_SET(map, n) (map[n >> 5] & (1 << (n & (WORD_WIDE - 1))))

#define BIT_CLR(map, n) (map[n >> 5] & ~(1 << (n & (WORD_WIDE - 1))))



int *
bitmap_init(int n)
{
	if(n < INT_MAX)
	{
		n += 1;
	}
	int sz = 1 + n / WORD_WIDE;
	int *map = (int *)malloc(sizeof(int) * sz);
	memset(map, 0, sz);
	return map;
}

int 
main()
{
#define N 1000000
	int *map = bitmap_init(N);
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
	for(int i = 0; i < 10; ++i)
	{
		while(1)
		{
			int n = rand() % N;
			if(!IS_SET(map, n))
			{
				BIT_SET(map, n);
				break;
			}
		}
	}
	for(int i = 0; i < N; ++i)
	{
		if(IS_SET(map, i))
		{
			printf("%d ", i);
		}
	}
	printf("\n");
	return 0;
}
