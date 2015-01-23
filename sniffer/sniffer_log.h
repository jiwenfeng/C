#ifndef __SNIFFER_LOG_H__
#define __SNIFFER_LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct sniffer_base;

int sniffer_log_init(struct sniffer_base *base);
void sniffer_log_report(const char *fmt, ...);
void sniffer_log_cleanup(FILE *fp);
void sniffer_log_write(struct sniffer_base *base, const char *fmt, ...);
char *now();

#define report(fmt, ...)	do{\
	sniffer_log_report("[%s] %s:%d "fmt"\n", now(), __FILE__, __LINE__, ##__VA_ARGS__);		\
}while(0)

#define fatal(fmt, ...)		do{\
	sniffer_log_report("[%s] %s:%d "fmt"\n", now(), __FILE__, __LINE__, ##__VA_ARGS__);		\
	exit(0);	\
}while(0)

#define log(base, fmt, ...)	do{\
	sniffer_log_write(base, "[%s] %s:%d "fmt"\n", now(), __FILE__, __LINE__, ##__VA_ARGS__);\
}while(0)

#endif
