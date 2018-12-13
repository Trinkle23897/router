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
};

struct TRtEntry {
	// TRtEntry *pstNext;
	in_addr addr; // addr
	in_addr mask; // mask
	in_addr nexthop;
	uint32_t metric;
	char *ifname;
};

struct TSockRoute {
	uint32_t uiPrefixLen;
	in_addr stIpPrefix;
	uint32_t uiIfindex;
	in_addr stNexthop;
	uint32_t uiCmd;
};

// void route_SendForward(uint32_t uiCmd, TRtEntry *pstRtEntry);
// void requestpkt_Encapsulate();
// void rippacket_Receive();
// void rippacket_Send(in_addr stSourceIp);
// void rippacket_Multicast(char *pcLocalAddr);
// void request_Handle(in_addr stSourceIp);
// void response_Handle(in_addr stSourceIp);
// void rippacket_Update();
// void routentry_Insert();
// void localinterf_GetInfo();
// void ripdaemon_Start();

#endif
