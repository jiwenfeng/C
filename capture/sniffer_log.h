#ifndef __SNIFFER_LOG_H__
#define __SNIFFER_LOG_H__

char *now();
void do_report(void *inst, const char *fmt, ...);

#define sniffer_report(inst, fmt, ...) do { do_report(inst, "[%s] "fmt"\n", now(), ##__VA_ARGS__); } while(0)

#define sniffer_error(fmt, ...) do { do_report(stderr, "\33[31m[Error] %s:%d "fmt "\33[0m\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(0)

#endif
