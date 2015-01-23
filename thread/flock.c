#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void *
routine(void *arg)
{
	char *prompt = (char *)arg;
	char *string;
	int len;
	string = (char *)malloc(128);
	if(string == NULL)
	{
		return NULL;
	}
	flockfile(stdin);
	flockfile(stdout);
	printf(prompt);
	if(fgets(string, 128, stdin) == NULL)
	{
		string[0] = '\0';
	}
	else
	{
		len = strlen(string);
		if(len > 0 && string[len - 1] == '\n')
		{
			string[len - 1] = '\0';
		}
	}
	funlockfile(stdout);
	funlockfile(stdin);
	return string;
}

int
main()
{
	pthread_t pid1, pid2, pid3;
	void *string;
	pthread_create(&pid1, NULL, routine, "Thread 1>");
	pthread_create(&pid2, NULL, routine, "Thread 2>");
	pthread_create(&pid3, NULL, routine, "Thread 3>");
	pthread_join(pid1, &string);
	free(string);
	pthread_join(pid2, &string);
	free(string);
	pthread_join(pid3, &string);
	free(string);
	return 0;
}
