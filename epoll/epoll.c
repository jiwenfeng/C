#include "epoll.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <assert.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int _epoll_loop(struct epoll_base *base, int timeout);

static int _epoll_accpet(struct epoll_base *base);

struct epoll_base *
init_epoll_instance()
{
	struct epoll_base *base = (struct epoll_base *)malloc(sizeof(struct epoll_base *));
	if(NULL == base)
	{
		return NULL;
	}
	memset(base , '\0', sizeof(struct epoll_base));
	int efd = epoll_create(1024);
	if(efd == -1)
	{
		return NULL;
	}
	base->efd = efd;
	return base;
}

static int
_epoll_accept(struct epoll_base *base)
{
	struct sockaddr addr;
	socklen_t len;
	int fd = accept(base->listen_fd, (struct sockaddr *)&addr,  &len);
	if(-1 == fd)
	{
		return -1;
	}
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN|EPOLLOUT;
	return epoll_ctl(base->efd, EPOLL_CTL_ADD, fd, &ev);
}

static int
_epoll_recv(struct epoll_base *base, int fd)
{
	char buf[1024] = {0};
	int n = recv(fd, buf, 1024, 0);
	if(n <= 0)
	{
		return epoll_ctl(base->efd, EPOLL_CTL_DEL, fd, NULL);
	}
	printf("%s\n", buf);
}

static int
_epoll_loop(struct epoll_base *base, int timeout)
{
	struct epoll_event events[1024] = {0};
	int ret = epoll_wait(base->efd, events, 1024, timeout);
	if(-1 == ret)
	{
		return -1;
	}
	int i = 0;
	for(i = 0; i < ret; ++i)
	{
		if(events[i].data.fd == base->listen_fd)
		{
			_epoll_accept(base);
		}
		else
		{
			if(events[i].events & EPOLLIN) // READ
			{
				_epoll_recv(base, events[i].data.fd);
			}
			if(events[i].events & EPOLLOUT) // WRITE
			{

			}
		}
	}
	return 0;
}

int
epoll_dispatch(struct epoll_base *base, int timeout)
{
	while(1)
	{
		int ret = _epoll_loop(base, timeout);
		if(ret == -1)
		{
			return -1;
		}
	}
}

void
destory_epoll_instance(struct epoll_base *base)
{
	close(base->efd);
	free(base);
	base = NULL;
}
