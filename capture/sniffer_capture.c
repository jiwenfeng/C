#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <errno.h>
#include "sniffer.h"
#include "sniffer_buffer.h"
#include "sniffer_capture.h"
#include "sniffer_log.h"

struct sniffer_capture
{
	int fd;
};

static int proc_tcp_frame(struct sniffer_base *base, const char *str, int n);
static int proc_udp_frame(struct sniffer_base *base, const char *str, int n);
static int proc_ip_frame(struct sniffer_base *base, const char *str, int n);
static int proc_ether_frame(struct sniffer_base *base, const char *str, int n);

static int
proc_tcp_frame(struct sniffer_base *base, const char *str, int n)
{
	struct iphdr *ip = (struct iphdr *)str;
	int offset = ip->ihl * 4;
	struct tcphdr *tcp = (struct tcphdr *)(str + offset);
	offset += tcp->doff * 4;
	unsigned short sp = ntohs(tcp->source);
	unsigned short dp = ntohs(tcp->dest);
	struct sniffer_config *cfg = base->cfg;
	if(sp != cfg->port && dp != cfg->port)
	{
		return 0;
	}
	if(tcp->rst || tcp->fin)
	{
		sniffer_buffer_delete(base->sb, ip->saddr, ip->daddr, sp, dp);
		return 0;
	}
	if(ntohs(ip->tot_len) == offset || offset == n)
	{
		return 0;
	}
	return sniffer_buffer_push(base->sb, ip->saddr, ip->daddr, sp, dp, str + offset, n - offset);
}

static int
proc_udp_frame(struct sniffer_base *cap, const char *str, int n)
{
	return 0;
}

static int
proc_ip_frame(struct sniffer_base *base, const char *str, int n)
{
	struct iphdr *ip = (struct iphdr *)str;
	if(ip->version != 4 || ip->ihl != 5)
	{
		return 0;
	}
#if 0
	if(ntohs(ip->tot_len) - sizeof(struct iphdr) - sizeof(struct tcphdr) == 0)
	{
		return 0;
	}
#endif
	struct sniffer_config *cfg = base->cfg;
	if(ip->saddr != cfg->s_addr && ip->daddr != cfg->s_addr)
	{
		return 0;
	}
	if(ip->daddr != cfg->d_addr && ip->daddr != cfg->s_addr)
	{
		return 0;
	}
	if(strncmp(cfg->proto, "TCP", 3) == 0 && ip->protocol == IPPROTO_TCP)
	{
		return proc_tcp_frame(base, str, n);
	}
	if(strncmp(cfg->proto, "UDP", 3) == 0 && ip->protocol == IPPROTO_UDP)
	{
		return proc_udp_frame(base, str, n);
	}
	return 0;
}

static int
proc_ether_frame(struct sniffer_base *base, const char *str, int n)
{
	struct ethhdr *eth = (struct ethhdr *)str;
	switch(ntohs(eth->h_proto))
	{
		case ETH_P_IP:
			return proc_ip_frame(base, str + sizeof(struct ethhdr), n - sizeof(struct ethhdr));
		default:
			break;
	}
	return 0;
}

struct sniffer_capture *
sniffer_capture_init()
{
	int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(-1 == fd)
	{
		sniffer_error("%s", strerror(errno));
		return NULL;
	}
	struct sniffer_capture *cap = (struct sniffer_capture *)malloc(sizeof(struct sniffer_capture));
	cap->fd = fd;
	return cap;
}

int
sniffer_capture_cap(struct sniffer_base *base)
{
	char buf[65535] = {0};
	struct sniffer_capture *cap = base->cap;
	int n = recvfrom(cap->fd, buf, 65535, 0, NULL, NULL);
	if(n < 0)
	{
		sniffer_error("%s\n", strerror(errno));
		return n;
	}
	return proc_ether_frame(base, buf, n);
}

void
sniffer_capture_destroy(struct sniffer_capture *cap)
{
	close(cap->fd);
	free(cap);
}
