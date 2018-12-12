#ifndef __MYRIP_H__
#define __MYRIP_H__

#include "defines.h"

struct TRipEntry {
	uint16_t usFamily;
	uint16_t usTag;
	in_addr stAddr;
	in_addr stPrefixLen;
	in_addr stNexthop;
	uint32_t uiMetric;
};

struct TRipPkt {
	uint8_t ucCommand;
	uint8_t ucVersion;
	uint16_t usZero; 
	TRipEntry RipEntries[RIP_MAX_ENTRY];
};

struct Interface {
	uint32_t addr;
	char* name;
};

struct TRtEntry {
	// TRtEntry *pstNext;
	in_addr stIpPrefix; 
	uint32_t uiPrefixLen;
	in_addr stNexthop;
	uint32_t uiMetric;
	char *pcIfname;
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
