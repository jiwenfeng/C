#include "sniffer_log.h"
#include "sniffer.h"
#include <time.h>

char *
now()
{
	static char buf[256] = {0};
	time_t tt = time(NULL);
	struct tm *t = localtime(&tt);
	strftime(buf, 256, "%F %T", t);
	return buf;
}


void 
sniffer_log_report(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	fflush(stdout);
	va_end(ap);
}

int
sniffer_log_init(struct sniffer_base *base)
{
	FILE *fp = fopen("sniffer_log.log", "a+");
	if(NULL == fp)
	{
		return -1;
	}
	base->log_handler = fp;
	return 0;
}

void 
sniffer_log_write(struct sniffer_base *base, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(base->log_handler, fmt, ap);
	fflush(base->log_handler);
	va_end(ap);
}

void 
sniffer_log_cleanup(FILE *fp)
{
	if(!fp)
	{
		return ;
	}
	fflush(fp);
	fclose(fp);
}
