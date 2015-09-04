#ifndef __SNIFFER_CAPTURE_H__
#define __SNIFFER_CAPTURE_H__

struct sniffer_capture;
struct sniffer_base;

struct sniffer_capture *sniffer_capture_init();
int sniffer_capture_cap(struct sniffer_base *base);
void sniffer_capture_destroy(struct sniffer_capture *cap);

#endif
