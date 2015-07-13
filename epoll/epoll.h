#ifndef __EPOLL_H__
#define __EPOLL_H__


typedef void(*epoll_cb)(void *arg, int event, int fd);

struct epoll_base
{
	int efd;
	int listen_fd;
};


struct epoll_base * init_epoll_instance();

int epoll_dispatch(struct epoll_base *base, int timeout);

void destory_epoll_instance(struct epoll_base *base);

#endif
