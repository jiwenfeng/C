#ifndef __SNIFFER_PARSE_H__
#define __SNIFFER_PARSE_H__

struct sniffer_base;
int sniffer_parse_init(struct sniffer_base *base);
int sniffer_parse_data(struct sniffer_base *base, const char *data, size_t ss);
void sniffer_parse_cleanup(void *handler);

#endif
