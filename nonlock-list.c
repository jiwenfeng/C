#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

struct message
{
	int val;
	pthread_t pid;
	struct message *next;
};

struct message_queue
{
	int lock;
	int idx;
	struct message *head;
	struct message *last;
};


struct message_queue *message_queue_init();
void message_queue_push(struct message_queue *mq);
void message_queue_destroy(struct message_queue *mq);
void message_queue_debug(struct message_queue *mq);

#ifdef __USE_LOCK__
#define THREAD_NUM 5
#define lock(q) while(__sync_lock_test_and_set(&q->lock, 1)){}
#define unlock(q) __sync_lock_release(&q->lock)
#else
#define THREAD_NUM 1
#define lock(q)
#define unlock(q)
#endif

#define LOOP_NUM 50000 / THREAD_NUM

int
getms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

struct message_queue *
message_queue_init()
{
	struct message_queue *mq = (struct message_queue *)malloc(sizeof(struct message_queue));
	mq->lock = 0;
	mq->idx = 0;
	mq->head = mq->last = NULL;
	return mq;
}

void
message_queue_destroy(struct message_queue *mq)
{
	struct message *head = mq->head;
	while(head != NULL)
	{
		struct message *msg = head;
		head = head->next;
		free(msg);
	}
	free(mq);
}

void
message_queue_push(struct message_queue *mq)
{
	struct message *msg = (struct message *)malloc(sizeof(struct message));
	msg->val = __sync_add_and_fetch(&mq->idx, 1);
	msg->next = NULL;
	msg->pid = pthread_self();
	lock(mq);
	if(mq->head == NULL)
	{
		mq->head = msg;
	}
	else
	{
		mq->last->next = msg;
	}
	mq->last = msg;
	unlock(mq);
}

void
message_queue_debug(struct message_queue *mq)
{
	struct message *msg = mq->head;
	while(msg != NULL)
	{
		printf("%u:%d\n", msg->pid, msg->val);
		msg = msg->next;
	}
}

void *
push_message(void *arg)
{
	struct message_queue *mq = (struct message_queue *)arg;
	int i = 0;
	while(i < LOOP_NUM)
	{
		message_queue_push(mq);
		++i;
	}
	return NULL;
}

int
main()
{
	struct message_queue *mq = message_queue_init();
	pthread_t pid[THREAD_NUM];
	int i = 0;
	for(i = 0; i < THREAD_NUM; ++i)
	{
		pthread_create(&pid[i], NULL, push_message, mq);
	}
	for(i = 0; i < THREAD_NUM; ++i)
	{
		pthread_join(pid[i], NULL);
	}
	message_queue_debug(mq);
	message_queue_destroy(mq);
	return 0;
}	
