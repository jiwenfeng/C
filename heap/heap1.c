#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LCHILD(i)	 (2 * (i) + 1)
#define RCHILD(i)	 (2 * (i) + 2)
#define PARENT(i)	 (((i) - 1) / 2)

void
swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void
fix(int a[], int i, int length)
{
	int L = LCHILD(i);
	int R = RCHILD(i);
	int largest = i;
	if(L < length && a[L] > a[largest])
	{
		largest = L;
	}
	if(R < length && a[R] > a[largest])
	{
		largest = R;
	}
	if(i != largest)
	{
		swap(&a[i], &a[largest]);
		fix(a, largest, length);
	}
}

void 
build_max_heap(int a[], int length)
{
	int i = length / 2 - 1;
	for(i; i >= 0; i--)
	{
		fix(a, i, length);
	}
}

void
heap_sort(int a[], int length)
{
	build_max_heap(a, length);
	int i = length - 1;
	for(i; i >= 0; i--)
	{
		swap(&a[0], &a[i]);
		fix(a, 0, i);
	}
}

void
print(int a[], int length)
{
	int i = 0;
	for(i = 0; i < length; i++)
	{
		printf("%d ", a[i]);
	}
	printf("\n");
}

int
main()
{
	int a[] = {23,423,2,35,4,536,4,2,1,234,5,67,8,9,3,221,230};
#define length sizeof(a) / sizeof(a[0])
	print(a, length);
	heap_sort(a, length);
	print(a, length);
	return 0;
}
