#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "epoll.h"

int
main()
{
	struct epoll_base *base = init_epoll_instance();
	if(NULL == base)
	{
		return ;
	}
	epoll_dispatch(base, -1);
	destory_epoll_instance(base);
	return 0;
}
