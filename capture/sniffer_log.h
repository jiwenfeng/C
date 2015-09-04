#ifndef __SNIFFER_LOG_H__
#define __SNIFFER_LOG_H__

char *now();
void report(void *inst, const char *fmt, ...);

#define sniffer_report(inst, fmt, ...) do { report(inst, "[%s] "fmt"\n", now(), ##__VA_ARGS__); } while(0)

#define sniffer_error(fmt, ...) do { report(stderr, "\33[31m[Error] %s:%d "fmt "\33[0m\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(0)

#endif
