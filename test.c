#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
display_array(int *a, int n)
{
	int i = 0;
	for(i = 0; i < n; ++i)
	{
		printf("%d ", a[i]);
	}
	printf("\n");
}

int 
partition(int *a, int left, int right)
{
	int i = left;
	int temp = a[i];
	while(left != right)
	{
		while(a[right] >= temp && right > left)
		{
			--right;
		}
		while(a[left] <= temp && left < right)
		{
			++left;
		}
		if(left < right)
		{
			int k = a[left];
			a[left] = a[right];
			a[right] = k;
		}
	}
	a[i] = a[left];
	a[left] = temp;
	return left;
}

void
quick_sort(int *a, int left, int right)
{
	if(left < right)
	{
		int r = partition(a, left, right);
		quick_sort(a, left, r - 1);
		quick_sort(a, r + 1, right);
	}
}

int
main()
{
	int a[] = {3, 12, 432, 53, 5, 32, 44, 53, 0, 1};
#define N sizeof(a) / sizeof(a[0])
	display_array(a, N);
	quick_sort(a, 0, N - 1);
	display_array(a, N);
	return 0;
}
