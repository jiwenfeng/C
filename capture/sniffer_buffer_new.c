#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct session
{
	ulong sa;
	ulong da;
	ushort sp;
	ushort dp;
	struct session *next;
	struct session *prev;
};

struct sniffer_buffer 
{

};
