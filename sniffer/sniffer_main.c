#include "sniffer.h"
#include "sniffer_parse.h"
#include "sniffer_config.h"
#include "sniffer_log.h"
#include "sniffer_buffer.h"


#ifdef __linux__
extern const struct sniffer_op linux_op;
extern const struct sniffer_thread linux_thread;
#endif

#ifdef WIN32
extern const struct sniffer_op win32_op;
#endif

static const struct sniffer_op *ops[] = {
#ifdef __linux__
	&linux_op,
#endif
#ifdef WIN32
	&win32_op,
#endif
};

static const struct sniffer_thread *threads [] = {
#ifdef __linux__
	&linux_thread,
#endif
#ifdef WIN32
#endif
};

#define SNIFFER_STANDBY		0x01
#define SNIFFER_RUNNING		0x02
#define SNIFFER_SHUTDOWN	0x03
#define SNIFFER_SUSPEND		0x04

static struct sniffer_base *g_base = NULL;

static struct sniffer_cfg *_sniffer_config_new();
static struct sniffer_base *_sniffer_base_init();
static int _sniffer_loop(struct sniffer_base *base);
static void _sniffer_cleanup(struct sniffer_base *base);

static struct sniffer_cfg *
_sniffer_config_new()
{
	if(-1 == open_config_file("./config"))
	{
		return NULL;
	}
	struct sniffer_cfg *cfg = (struct sniffer_cfg *)malloc(sizeof(struct sniffer_cfg));
	memset(cfg, '\0', sizeof(struct sniffer_cfg));
	cfg->port = get_config("sniffer_port");
	close_config_file();
	return cfg;
}

static struct sniffer_base *
_sniffer_base_init()
{
	struct sniffer_base *base = (struct sniffer_base *)malloc(sizeof(struct sniffer_base));
	if(!base)
	{
		return NULL;
	}
	base->status = SNIFFER_STANDBY;
	base->cfg = _sniffer_config_new();
	if(NULL == base->cfg)
	{
		_sniffer_cleanup(base);
		return NULL;
	}
	base->sb_send = sniffer_buffer_init();
	base->sb_recv = sniffer_buffer_init();
	base->sb_cur = NULL;
	base->sb_output = sniffer_buffer_init();
	if(-1 == sniffer_parse_init(base))
	{
		_sniffer_cleanup(base);
		return NULL;
	}
	if(-1 == sniffer_log_init(base))
	{
		_sniffer_cleanup(base);
		return NULL;
	}
	int i;
	for(i = 0; i < sizeof(ops) / sizeof(ops[0]); ++i)
	{
		const struct sniffer_op *o = ops[i];
		base->op = o;
		void *handler = o->init(base);
		if(!handler)
		{
			_sniffer_cleanup(base);
			return NULL;
		}
		base->op_handler = handler;
		break;
	}
	for(i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i)
	{
		const struct sniffer_thread *thread = threads[i];
		base->thread = thread;
		void *handler = thread->init(base);
		if(!handler)
		{
			_sniffer_cleanup(base);
			return NULL;
		}
		base->thread_handler = handler;
	}
	return base;
}

static void
_sniffer_cleanup(struct sniffer_base *base)
{
	sniffer_buffer_cleanup(base->sb_send);
	sniffer_buffer_cleanup(base->sb_recv);
	sniffer_buffer_cleanup(base->sb_output);
	sniffer_parse_cleanup(base->parse_handler);
	sniffer_log_cleanup(base->log_handler);
	if(base->op && base->op->cleanup)
	{
		base->op->cleanup(base->op_handler);
	}
	if(base->thread && base->thread->cleanup)
	{
		base->thread->cleanup(base->thread_handler);
	}
	free(base->cfg);
	free(base);
	g_base = NULL;
}

static int
_sniffer_loop(struct sniffer_base *base)
{
	if(base->status == SNIFFER_RUNNING)
	{
		return 0;
	}
	base->status = SNIFFER_RUNNING;
	// create thread here to sniffer data
	while(1)
	{
		if(base->status == SNIFFER_SHUTDOWN)
		{
			break;
		}
		if(base->status == SNIFFER_SUSPEND)
		{
			continue;
		}
		if(-1 == base->op->dispatch(base))
		{
			break;
		}
		int ss = sniffer_buffer_size(base->sb_cur);
		if(0 == ss)
		{
			continue;
		}
		const char *data = sniffer_buffer_peekup(base->sb_cur);
		int n = sniffer_parse_data(base, data, ss);
		if(-1 == n)
		{
			break;
		}
		sniffer_buffer_remove(base->sb_cur, n);
		ss = sniffer_buffer_size(base->sb_output);
		if(ss == 0)
		{
			continue;
		}
		const char *str = sniffer_buffer_peekup(base->sb_output);
		report("%s", str);
		sniffer_buffer_remove(base->sb_output, ss);
	}
	_sniffer_cleanup(base);
	return 0;
}

int
sniffer_loop(struct sniffer_base *base)
{
	return _sniffer_loop(base);
}

void 
sniffer_suspend(struct sniffer_base *base)
{
	base->status = SNIFFER_SUSPEND;
}

void
sniffer_resume(struct sniffer_base *base)
{
	base->status = SNIFFER_RUNNING;
}

void
sniffer_shutdown(struct sniffer_base *base)
{
	base->status = SNIFFER_SHUTDOWN;
}

struct sniffer_base *
sniffer_init()
{
	struct sniffer_base *base = _sniffer_base_init();
	if(!base)
	{
		return NULL;
	}
	g_base = base;
	return base;
}

#ifndef __SNIFFER_WINDOW__
int
main(int argc, char *argv[])
{
	struct sniffer_base *base = sniffer_init();
	if(!base)
	{
		return 0;
	}
	sniffer_loop(base);
	return 0;
}
#endif
