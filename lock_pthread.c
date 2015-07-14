#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define MAGIC 0x12FA85B

struct pthread_rwlock
{
	pthread_mutex_t mutex;
	pthread_cond_t read;
	pthread_cond_t write;
	int magic;
	int status;
	int nread;
	int nwrite;
};

void 
cancel_rdlock(void *arg)
{
	struct pthread_rwlock *lock = (struct pthread_rwlock *)arg;
	--lock->nread;
	pthread_mutex_unlock(&lock->mutex);
}

void
cancel_wrlock(void *arg)
{
	struct pthread_rwlock *lock = (struct pthread_rwlock *)arg;
	--lock->nwrite;
	pthread_mutex_unlock(&lock->mutex);
}

void
rwlock_init(struct pthread_rwlock *lock)
{
	memset(lock, '\0', sizeof(struct pthread_rwlock));
	pthread_mutex_init(&lock->mutex, NULL);
	pthread_cond_init(&lock->read, NULL);
	pthread_cond_init(&lock->write, NULL);
	lock->status = 0;
	lock->nread = 0;
	lock->nwrite = 0;
	lock->magic = MAGIC;
}

void
rwlock_destroy(struct pthread_rwlock *lock)
{
	if(lock->magic != MAGIC)
	{
		return ;
	}
	if(lock->status != 0 || lock->nread != 0 || lock->nwrite != 0)
	{
		return ;
	}
	pthread_mutex_destroy(&lock->mutex);
	pthread_cond_destroy(&lock->read);
	pthread_cond_destroy(&lock->write);
	lock->magic = 0;
}

void
rwlock_rdlock(struct pthread_rwlock *lock)
{
	if(lock->magic != MAGIC)
	{
		return ;
	}
	pthread_mutex_lock(&lock->mutex);
	while(lock->status == -1 || lock->nwrite > 0)
	{
		++lock->nread;
		pthread_cleanup_push(cancel_rdlock, lock);
		pthread_cond_wait(&lock->read, &lock->mutex);
		pthread_cleanup_pop(0);
		--lock->nread;
	}
	++lock->status;
	pthread_mutex_unlock(&lock->mutex);
}

void
rwlock_wrlock(struct pthread_rwlock *lock)
{
	if(lock->magic != MAGIC)
	{
		return ;
	}
	pthread_mutex_lock(&lock->mutex);
	while(lock->status != 0)
	{
		++lock->nwrite;
		pthread_cleanup_push(cancel_wrlock, lock);
		pthread_cond_wait(&lock->write, &lock->mutex);
		pthread_cleanup_pop(0);
		--lock->nwrite;
	}
	lock->status = -1;
	pthread_mutex_unlock(&lock->mutex);
}

int 
rwlock_try_rdlock(struct pthread_rwlock *lock)
{
	if(lock->magic != MAGIC)
	{
		return -2;
	}
	pthread_mutex_lock(&lock->mutex);
	if(lock->status == -1 || lock->nwrite > 0)
	{
		return -1;
	}
	++lock->status;
	pthread_mutex_unlock(&lock->mutex);
	return 0;
}

int 
rwlock_try_wrlock(struct pthread_rwlock *lock)
{
	if(lock->magic != MAGIC)
	{
		return -2;
	}
	pthread_mutex_lock(&lock->mutex);
	if(lock->status != 0)
	{
		return -1;
	}
	lock->status = -1;
	pthread_mutex_unlock(&lock->mutex);
	return 0;
}

void 
rwlock_unlock(struct pthread_rwlock *lock)
{
	if(lock->magic != MAGIC)
	{
		return ;
	}
	pthread_mutex_lock(&lock->mutex);
	if(lock->status > 0)
	{
		--lock->status;
	}
	else
	{
		if(lock->status == -1)
		{
			lock->status = 0;
		}
		else
		{
			return;
		}
	}
	if(lock->status == 0)
	{
		if(lock->nwrite > 0)
		{
			pthread_cond_signal(&lock->write);
		}
	}
	else
	{
		if(lock->nread > 0)
		{
			pthread_cond_broadcast(&lock->read);
		}
	}

	pthread_mutex_unlock(&lock->mutex);
}

int
main()
{

	return 0;
}
