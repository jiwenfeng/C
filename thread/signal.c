#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

struct handler
{
	int status;
	sigset_t mask;
	pthread_mutex_t lock;
	pthread_cond_t ready;
};

struct handler *
handler_init()
{
	struct handler *h = (struct handler *)malloc(sizeof(struct handler));
	h->status = 0;
	sigemptyset(&h->mask);
	pthread_mutex_init(&h->lock, NULL);
	pthread_cond_init(&h->ready, NULL);
	return h;
}

void *
rtn_fun(void *arg)
{
	struct handler *h = (struct handler *)arg;
	int err, signo;
	while(1)
	{
		err = sigwait(&h->mask, &signo);
		if(err != 0)
		{
			printf("sigwait error:%d\n", err);
			return NULL;
		}
		switch(signo)
		{
			case SIGINT:
				printf("interrupt\n");
				break;
			case SIGQUIT:
				pthread_mutex_lock(&h->lock);
				h->status = 1;
				pthread_cond_signal(&h->ready);
				pthread_mutex_unlock(&h->lock);
				return NULL;
			default:
				printf("expected signal\n");
				break;
		}
	}
	return NULL;
}

int
main()
{
	struct handler *h = handler_init();
	sigset_t old;
	sigaddset(&h->mask, SIGINT);
	sigaddset(&h->mask, SIGQUIT);

	if(pthread_sigmask(SIG_BLOCK, &h->mask, &old) != 0)
	{
		printf("sig mask failed\n");
		return 0;
	}

	pthread_t pid;
	if(0 != pthread_create(&pid, NULL, rtn_fun, h))
	{
		printf("create thread failed\n");
		return 0;
	}

	pthread_mutex_lock(&h->lock);
	while(h->status == 0)
	{
		pthread_cond_wait(&h->ready, &h->lock);
	}
	pthread_mutex_unlock(&h->lock);

	if(0 != sigprocmask(SIG_SETMASK, &old, NULL))
	{
		printf("sigprocmask failed\n");
	}

	return 0;
}
