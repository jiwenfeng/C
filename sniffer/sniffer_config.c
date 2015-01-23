#include "sniffer_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static FILE *fp = NULL;

int
open_config_file(const char *path)
{
	fp = fopen(path, "r");
	if(!fp)
	{
		return -1;
	}
	return 0;
}

int
get_config(const char *key)
{
	assert(fp);
	rewind(fp);
	size_t ss = strlen(key);
	char buf[1024] = {0};
	while(NULL != fgets(buf, 1024, fp))
	{
		if(memcmp(buf, key, ss) != 0)
		{
			continue;
		}
		char *str = strstr(buf, "=");
		if(NULL == str)
		{
			continue;
		}
		while(*str == ' ' || *str == '=')
		{
			str++;
		}
		int ret = 0;
		while(*str >= '0' && *str <= '9')
		{
			ret = ret * 10 + (*str - '0');
			str++;
		}
		return ret;
	}
	return -1;
}

const char *
get_config_string(const char *key)
{
	assert(fp);
	rewind(fp);
	size_t ss = strlen(key);
	char buf[1024] = {0};
	while(NULL != fgets(buf, 1024, fp))
	{
		if(memcmp(key, buf, ss) != 0)
		{
			continue;
		}
		char *str = strstr(buf, "=");
		if(NULL == str)
		{
			continue;
		}
		while(*str == ' ' || *str == '=')
		{
			str++;
		}
		while(*str != ' ' && *str != '\0' && *str != '#')
		{
			
		}
	}
	return NULL;
}

void
close_config_file()
{
	if(!fp)
	{
		return ;
	}
	fclose(fp);
	fp = NULL;
}
