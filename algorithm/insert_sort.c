#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void 
InsertSort(int a[], int n)
{
	int i, j;
	for(i = 1; i < n; ++i)
	{
		int k = a[i];
		for(j = i - 1; j >= 0; --j)
		{
			if(a[j] < k)
			{
				break;
			}
			a[j + 1] = a[j];
		}
		a[j + 1] = k;
	}
}

void
Display(int a[], int n)
{
	int i = 0;
	for(i = 0; i < n; ++i)
	{
		printf("%-5d", a[i]);
	}
	printf("\n");
}

int 
main()
{
	int a[] = {12, 2, 56, 21, 9, 32, 90, 89, 30};
#define N sizeof(a) / sizeof(a[0])
	Display(a, N);

	InsertSort(a, N);

	Display(a, N);
	return 0;
}
