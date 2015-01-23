#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define POOL_RUNNING			0x01
#define POOL_SHUTDOWN			0x02

typedef struct task_tag
{
	struct task_tag *next;
	void *(*callback)(void *);
	void *arg;
}task;

typedef struct pool_tag
{
	task *task_list;
	int cur;
	int state;
	int total;
	pthread_t *pidlist;
	pthread_mutex_t lock;
	pthread_cond_t ready;
}pool;


static void *pool_routine(void *arg)
{
	pool *p = (pool *)arg;
	while(1)
	{
		pthread_mutex_lock(&(p->lock));
		while((p->state == POOL_RUNNING) && (p->cur == 0))
		{
			pthread_cond_wait(&p->ready, &p->lock);
		}
		if(p->state == POOL_SHUTDOWN)
		{
			pthread_mutex_unlock(&p->lock);
			printf("%lu:destroyed\n", pthread_self());
			pthread_exit(0);
		}
		task *t = p->task_list;
		p->task_list = p->task_list->next;
		p->cur--;
		pthread_mutex_unlock(&p->lock);
		t->callback(t->arg);
		free(t);
	}
}

void add_task(pool *h, void *(*callback)(void *), void *arg)
{
	if(h->state == POOL_SHUTDOWN)
	{
		return;
	}
	task *t = (task *)malloc(sizeof(task));
	t->callback = callback;
	t->arg = arg;
	t->next = NULL;
	pthread_mutex_lock(&h->lock);
	if(!h->task_list)
	{
		h->task_list = t;
	}
	else
	{
		task *head = h->task_list;
		
		while(head->next)
		{
			head = head->next;
		}
		head->next = t;
	}
	h->cur++;
	pthread_mutex_unlock(&h->lock);
	pthread_cond_signal(&h->ready);
}

void pool_destroy(pool *h)
{
	if(h->state == POOL_SHUTDOWN)
	{
		return;
	}
	h->state = POOL_SHUTDOWN;
	pthread_cond_broadcast(&(h->ready));
	int i;
	//waiting for all thread finished
	for(i = 0; i < h->total; i++)
	{
		pthread_join(h->pidlist[i], NULL);
	}
	while(h->task_list)
	{
		task *tmp = h->task_list;
		h->task_list = h->task_list->next;
		free(tmp);
	}
	pthread_mutex_destroy(&(h->lock));
	pthread_cond_destroy(&(h->ready));
	free(h->pidlist);
	free(h);
}

pool * pool_init(int num)
{
	pool *p = (pool *)malloc(sizeof(pool));
	if(!p)
	{
		return NULL;
	}
	p->total = num;
	p->cur = 0;
	p->state = POOL_RUNNING;
	p->pidlist = (pthread_t *)malloc(sizeof(pthread_t) * num);
	p->task_list = NULL;
	pthread_mutex_init(&(p->lock), NULL);
	pthread_cond_init(&(p->ready), NULL);
	int i;
	for(i = 0; i < num; i++)
	{
		pthread_create(&p->pidlist[i], NULL, pool_routine, p);
	}
	return p;
}

void *callback(void *p)
{
	printf("%lu:process this callback\n", pthread_self());
	return 0;
}

int main()
{
	pool *p = pool_init(100);

	sleep(10);
	pool_destroy(p);
	return 0;
}


