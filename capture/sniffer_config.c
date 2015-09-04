#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <errno.h>
#include "sniffer.h"
#include "sniffer_log.h"
#include "sniffer_config.h"

static int readint(FILE *fp, const char *key);
static char *readstr(FILE *fp, const char *key);

static int
readint(FILE *fp, const char *key)
{
	rewind(fp);
	char buf[1024] = {0}, *ptr = NULL;
	size_t len = strlen(key);
	while(fgets(buf, 1024, fp) != NULL)
	{
		if(buf[0] != '#' && strncmp(buf, key, len) == 0)
		{
			ptr = buf;
			break;
		}
	}
	if(NULL == ptr)
	{
		return 0;
	}
	char *pos = strstr(ptr, "=");
	int n = 0;
	while(pos != NULL && *pos != '\0' && *pos != '\n')
	{
		if(isdigit(*pos))
		{
			n = n * 10 + *pos - '0';
		}
		++pos;
	}
	return n;
}

static char *
readstr(FILE *fp, const char *key)
{
	rewind(fp);
	size_t len = strlen(key);
	char buf[1024] = {0}, *ptr = NULL;
	while(fgets(buf, 1024, fp) != NULL)
	{
		if(buf[0] != '#' && strncmp(buf, key, len) == 0)
		{
			ptr = buf;
			break;
		}
	}
	if(NULL == ptr)
	{
		return NULL;
	}
	char *pos = strstr(ptr, "=");
	if(NULL == pos)
	{
		return NULL;
	}
	pos = strstr(pos, "\"");
	if(NULL == pos)
	{
		return NULL;
	}
	++pos;
	size_t vlen = strlen(pos) - 1;
	char *value = (char *)malloc(vlen);
	memset(value, '\0', vlen);
	memcpy(value, pos, vlen - 1);
	return value;
}

struct sniffer_config *
sniffer_config_init(const char *path)
{
	FILE *fp = fopen(path, "r");
	if(NULL == fp)
	{
		sniffer_error("%s", strerror(errno));
		return NULL;
	}
	struct sniffer_config *cfg = (struct sniffer_config *)malloc(sizeof(struct sniffer_config));
	memset(cfg, '\0', sizeof(struct sniffer_config));
	cfg->port = readint(fp, "port");
	cfg->proto = readstr(fp, "proto");
	char *s_addr = readstr(fp, "src_addr");
	if(NULL == s_addr)
	{
		fclose(fp);
		sniffer_error("need src_addr key");
		return NULL;
	}
	cfg->s_addr = inet_addr(s_addr);
	free(s_addr);
	char *d_addr = readstr(fp, "dst_addr");
	if(NULL == d_addr)
	{
		fclose(fp);
		sniffer_error("need dst_addr key");
		return NULL;
	}
	cfg->d_addr = inet_addr(d_addr);
	free(d_addr);
	cfg->decode = readstr(fp, "decode");
	char *log_path = readstr(fp, "log");
	cfg->stream = log_path == NULL ? stdout : fopen(log_path, "w+");
	if(NULL == cfg->stream)
	{
		fclose(fp);
		sniffer_error("Could not open stream");
		return NULL;
	}
	fclose(fp);
	return cfg;
}

void
sniffer_config_destroy(struct sniffer_config *cfg)
{
	if(cfg->proto != NULL)
	{
		free(cfg->proto);
	}
	if(cfg->decode != NULL)
	{
		free(cfg->decode);
	}
	if(cfg->stream != stdout)
	{
		fclose(cfg->stream);
	}
	free(cfg);
}
