#ifndef __ARP__
#define __ARP__

#include "lookuproute.h"

struct arpmac
{
	unsigned char *mac;
	uint32_t index;
};

int32_t arpGet(arpmac*, nextaddr*);

#endif