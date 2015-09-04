#ifndef __SNIFFER_CONFIG_H__
#define __SNIFFER_CONFIG_H__


struct sniffer_config;

struct sniffer_config *sniffer_config_init(const char *path);

void sniffer_config_destroy(struct sniffer_config *cfg);

#endif
