#ifndef __RIP_H__
#define __RIP_H__

#include "defines.h"

struct TRipEntry {
	uint16_t family;
	uint16_t tag;
	in_addr addr;
	in_addr mask;
	in_addr nexthop;
	uint32_t metric; // little endian only
};

struct TRipPkt {
	uint8_t cmd;
	uint8_t ver;
	uint16_t zero; 
	TRipEntry entries[RIP_MAX_ENTRY];
};

struct Interface {
	uint32_t addr;
	char* name;
	bool activate;
};

struct TRtEntry {
	in_addr addr;
	in_addr mask;
	in_addr nexthop;
	uint32_t metric;
	char *ifname;
};

void rip_sendpkt(uint8_t*, uint32_t, uint32_t, uint16_t);
void rip_timeout_handler(uint32_t);
void rip_recv_handler(TRipPkt*, int32_t, uint32_t);
void* rip_recvpkt(void*);
void rip_timeout();
void* count_30s(void*);
void get_local_info(bool);
uint32_t lookfor24(uint32_t);
void start_rip();

extern std::vector<TRtEntry> rip_table;
extern std::vector<Interface> iface;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define fillEntry(src, fam, tag_, addr_, mask_, nxth, metr) \
	src.family = fam;\
	src.tag = tag_;\
	src.addr.s_addr = addr_;\
	src.mask = mask_;\
	src.nexthop.s_addr = 0;\
	src.metric = htonl(metr)

#endif
