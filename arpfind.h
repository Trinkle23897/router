#ifndef __ARP__
#define __ARP__

#include "defines.h"

struct arpmac
{
	unsigned char * mac;
	uint32_t index;
};

int32_t arpGet(arpmac *srcmac, char *ifname, char *ipStr);

#endif