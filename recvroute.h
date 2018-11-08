#ifndef __RECVROUTE__
#define __RECVROUTE__

#include "defines.h"

struct selfroute
{
	uint8_t prefixlen;
	in_addr prefix;
	uint32_t ifindex;
	in_addr nexthop;
	uint32_t cmdnum;
	char ifname[IF_NAMESIZE];
};

int32_t static_route_get(selfroute *selfrt);

#endif