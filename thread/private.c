#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_key_t	key;
long count = 0;

struct private_t
{
	pthread_t pid;
	const char *str;
};

void
thread_key_destructor(void *arg)
{
	struct private_t *p = (struct private_t *)arg;
	printf("Thread \"%s\" exiting\n", p->str);
	free(p);
	pthread_mutex_lock(&mutex);
	count--;
	if(count <= 0)
	{
		pthread_key_delete(key);
	}
	pthread_mutex_unlock(&mutex);
}

void *
thread_key_get()
{
	void *value = pthread_getspecific(key);
	if(NULL == value)
	{
		value = malloc(sizeof(struct private_t));
		pthread_setspecific(key, value);
	}
	return value;
}

void *
routine(void *arg)
{
	struct private_t *p = (struct private_t *)thread_key_get();
	p->pid = pthread_self();
	p->str = (char *)arg;
	printf("Thread \"%s\" staring\n", p->str);
	return NULL;
}

int
main()
{
	pthread_t pid1, pid2;
	pthread_key_create(&key, thread_key_destructor);
	struct private_t *p = thread_key_get();
	p->pid = pthread_self();
	p->str = "Main Thread";
	count = 3;
	pthread_create(&pid1, NULL, routine, "Thread 1");
	pthread_create(&pid2, NULL, routine, "Thread 2");

	pthread_join(pid1, NULL);
	pthread_join(pid2, NULL);

	pthread_exit(NULL);
	return 0;
}
