#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sniffer.h"
#include "sniffer_log.h"
#include "sniffer_module.h"

struct sniffer_module
{
	void *(*create)();
	int (*decode)(void *, const char *, int, char *, int);
	void (*destroy)(void *);
	void *inst;
	void *handle;
};


struct sniffer_module *
sniffer_module_init(const char *dlpath)
{
	void *handle = dlopen(dlpath, RTLD_NOW|RTLD_GLOBAL);
	if(NULL == handle)
	{
		sniffer_error("%s", dlerror());
		return NULL;
	}
	void *create = dlsym(handle, "create");
	if(NULL == create)
	{
		dlclose(handle);
		sniffer_error("%s", dlerror());
		return NULL;
	}
	void *decode = dlsym(handle, "decode");
	if(NULL == decode)
	{
		dlclose(handle);
		sniffer_error("%s", dlerror());
		return NULL;
	}
	void *destroy = dlsym(handle, "destroy");
	if(NULL == destroy)
	{
		dlclose(handle);
		sniffer_error("%s", dlerror());
		return NULL;
	}
	struct sniffer_module *sm = (struct sniffer_module *)malloc(sizeof(struct sniffer_module));
	if(NULL == sm)
	{
		dlclose(handle);
		sniffer_error("alloc memory failed");
		return NULL;
	}
	memset(sm, '\0', sizeof(struct sniffer_module));
	sm->create = create;
	sm->decode = decode;
	sm->destroy = destroy;
	sm->handle = handle;
	sm->inst = sm->create();
	return sm;
}

int
sniffer_module_callback(struct sniffer_module *sm, const char *str, int sz, char *buffer, int buflen)
{
	return sm->decode(sm->inst, str, sz, buffer, buflen);
}

void 
sniffer_module_destroy(struct sniffer_module *sm)
{
	if(NULL == sm)
	{
		return ;
	}
	if(sm->destroy)
	{
		sm->destroy(sm->inst);
	}
	if(sm->handle)
	{
		dlclose(sm->handle);
	}
	free(sm);
}
