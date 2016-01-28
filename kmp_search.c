#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

static void
compute_prefix(const char *p, int *next, int l)
{
	int k = 0;
	next[0] = 0;
	int i;
	for(i = 1; i < l; ++i)
	{
		while(k > 0 && p[k] != p[i])
		{
			k = next[k - 1];
		}
		if(p[k] == p[i])
		{
			k = k + 1;
		}
		next[i] = k;
	}
}

static int
kmp_search(const char *text, const char *p)
{
	int l1 = strlen(text);
	int l2 = strlen(p);
	if(l2 > l1)
	{
		return -1;
	}
	int *next = (int *)malloc(sizeof(int) * l2);
	memset(next, '\0', sizeof(int) * l2);
	compute_prefix(p, next, l2);
	int i = 0, j = 0;
	for(i = 0; i < l1; ++i)
	{
		if(p[j] == text[i])
		{
			++j;
			if(j == l2)
			{
				free(next);
				return i - j + 1;
			}
			continue;
		}
		j = j > 0 ? next[j - 1] + 1 : 0;
	}
	free(next);
	return -1;
}

int 
main()
{
	const char *text1 = "helloworldasdcrwiqurh234lkjalncakdnfklajhtiuaozhanzhehaohangtianfeijishenkongbaozhaduiyuzilingsakdfjelu5rhog3u5hfa;ldscnhjzhangyanhaahhatianzhanzhehaoasdlfi3qupojfc;dskcn4wuo5yxigusdfjhaidcsjdcnaiugbtoiu3b;nackeou45gqpadnczx,cnvbwuh efruhr;wlejfadknckljgtb4qi5bm adfjgvlksfadaabababadtnasdfurebsadfnbcndirnortncdalsdabababacahaiuhfruaceuab";
	const char *text2 = "abababababababababababababababababababababababababababababababababababababababababababababababababababababababababaababaababababababababababababababababababababababababababbbbbbbbbbbbbbbbbabababbababababababababababacbababababababa";
	const char *p = "ababac";
	int ret = kmp_search(text, p);
	if(ret != -1)
	{
		printf("search [%s] in [%s] at [%d]\n", p, text, ret);
	}
	else
	{
		printf("[%s] is not in the [%s]\n", p, text);
	}
	return 0;
}
