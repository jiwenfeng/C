#ifndef __SNIFFER_BUFFER_H__
#define __SNIFFER_BUFFER_H__

#include <stdint.h>

struct iphdr;
struct sniffer_buffer;

#ifndef ulong
#define ulong unsigned long
#endif

#ifndef ushort
#define ushort unsigned short
#endif

struct sniffer_buffer *sniffer_buffer_init();
int sniffer_buffer_push(struct sniffer_buffer *self, ulong sa, ulong da, ushort sp, ushort dp, const char *data, int len);
char *sniffer_buffer_peek(struct sniffer_buffer *self, int *sz);
void sniffer_buffer_remove(struct sniffer_buffer *self, char *str, int len);
void sniffer_buffer_destroy(struct sniffer_buffer *self);
int sniffer_buffer_delete(struct sniffer_buffer *self, ulong sa, ulong da, ushort sp, ushort dp);

#undef ulong
#undef ushort

#endif
