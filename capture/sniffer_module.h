#ifndef __SNIFFER_MODULE_H__
#define __SNIFFER_MODULE_H__

struct sniffer_module;

struct sniffer_module *sniffer_module_init(const char *dlpath);
int sniffer_module_callback(struct sniffer_module *sm, const char *str, int sz, char *buffer, int buflen);
void sniffer_module_destroy(struct sniffer_module *sm);

#endif
