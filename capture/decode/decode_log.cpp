#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include "decode_log.h"

char *
now()
{
	time_t tt = time(NULL);
	struct tm *tm = localtime(&tt);
	static char buf[128] = {0};
	strftime(buf, 128, "%F %T", tm);
	return buf;
}

void
do_report(void *inst, const char *fmt, ...)
{
	FILE *stream = (FILE *)inst;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stream, fmt, ap);
	fflush(stream);
	va_end(ap);
}
