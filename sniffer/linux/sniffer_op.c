#include <linux/tcp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <errno.h>
#include "../sniffer.h"
#include "../sniffer_log.h"
#include "../sniffer_buffer.h"

static void* _linux_op_init(struct sniffer_base *base);
static int _linux_op_dispatch(struct sniffer_base *base);
static void _linux_op_cleanup(void *handler);
static void _proc_ip(struct sniffer_base *base, const char *str, int datalen);
static void _proc_tcp(struct sniffer_base *base, const char *str, int datalen);
static void _proc_udp(struct sniffer_base *base, const char *str, int datalen);
static void _proc_icmp(struct sniffer_base *base, const char *str, int datalen);
static void _proc_igmp(struct sniffer_base *base, const char *str, int datalen);

#define DATALEN 65535

struct op
{
	int fd;
};

const struct sniffer_op linux_op = {
	_linux_op_init,
	_linux_op_dispatch,
	_linux_op_cleanup
};

static void *
_linux_op_init(struct sniffer_base *base)
{
	struct op *op = (struct op *)malloc(sizeof(struct op));
	if(!op)
	{
		return NULL;
	}
	int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(-1 == fd)
	{
		report("%d:%s", errno, strerror(errno));
		free(op);
		return NULL;
	}
	op->fd = fd;
	return op;
}

// 处理TCP数据包
static void 
_proc_tcp(struct sniffer_base *base, const char *str, int datalen)
{
	struct tcphdr *tcp = (struct tcphdr *)str;
	struct sniffer_cfg *cfg = (struct sniffer_cfg *)base->cfg;
	datalen -= tcp->doff * 4;
	if(datalen <= 0)
	{
		return;
	}
	if(ntohs(tcp->source) == cfg->port)
	{
		sniffer_buffer_push(base->sb_recv, str + tcp->doff * 4, datalen);
		base->sb_cur = base->sb_recv;
		return;
	}
	if(ntohs(tcp->dest) == cfg->port)
	{
		sniffer_buffer_push(base->sb_send, str + tcp->doff * 4, datalen);
		base->sb_cur = base->sb_send;
		return ;
	}
}

// 处理UDP数据包
static void
_proc_udp(struct sniffer_base *base, const char *str, int datalen)
{
}

// 处理ICMP数据包
static void
_proc_icmp(struct sniffer_base *base, const char *str, int datalen)
{
}

// 处理IGMP数据包
static void
_proc_igmp(struct sniffer_base *base, const char *str, int datalen)
{
}

static void 
_proc_ip(struct sniffer_base *base, const char *str, int datalen)
{
	struct iphdr *ip = (struct iphdr *)str;
	size_t len = ip->ihl * 4;
	switch(ip->protocol)
	{
		case IPPROTO_TCP:
			_proc_tcp(base, str + len, datalen - len);
			break;
		case IPPROTO_UDP:
			_proc_udp(base, str + len, datalen - len);
			break;
		case IPPROTO_ICMP:
			_proc_icmp(base, str + len, datalen - len);
			break;
		case IPPROTO_IGMP:
			_proc_igmp(base, str + len, datalen - len);
			break;
		default:
			break;
	}
}


static int
_linux_op_dispatch(struct sniffer_base *base)
{
	struct op *op = (struct op *)base->op_handler;
	char buf[DATALEN] = {0};
	int n = recvfrom(op->fd, buf, DATALEN, 0, NULL, NULL);
	if(n <= 0)
	{
		return 0;
	}
	struct ethhdr *eth = (struct ethhdr *)buf;
	switch(ntohs(eth->h_proto))
	{
		case ETH_P_IP:
			_proc_ip(base, buf + sizeof(struct ethhdr), n - sizeof(struct ethhdr));
			break;
		case ETH_P_ARP:
			break;
		default:
			break;
	}
	return 0;
}

static void
_linux_op_cleanup(void *handler)
{
	if(!handler)
	{
		return ;
	}
	struct op *op = (struct op *)handler;
	close(op->fd);
	free(op);
	return ;
}
