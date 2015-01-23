#ifndef __SNIFFER_CONFIG_H__
#define __SNIFFER_CONFIG_H__

int open_config_file(const char *path);
void close_config_file();
int get_config(const char *key);

#endif
