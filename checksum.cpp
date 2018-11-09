#include "checksum.h"

/*
 * 参数1是指向IP数据包头的指针ip_recvpkt，参数2是IP数据包的校验和
 */
int32_t check_sum(ip* iphd)
{
	uint16_t csum = count_check_sum(iphd, false);
	return csum == iphd->ip_sum ? 1 : 0;
}

/*
 * 参数是指向IP数据包头的指针ip_recvpkt
 * 注意：重新计算报文校验和前，ttl值减1
 * cf. http://www.cs.binghamton.edu/~steflik/cs455/rawip.txt
 */
uint16_t count_check_sum(ip* iphd, bool decline)
{
	uint16_t oldsum = iphd->ip_sum;
	iphd->ip_sum = 0;
	if (decline)
		iphd->ip_ttl -= 1;
	uint16_t* buf = (uint16_t*) iphd;
	uint32_t sum = 0;
	for (int32_t len = 0; len < IP_HEADER_LEN; len += 2)
		sum += *buf++;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += sum >> 16;
	iphd->ip_sum = oldsum;
	return ~sum;
}
/*
0x45 0x00 0x00 0x3d 0x6f 0xc7 0x00 0x00 0xff 0x11 0x84 0x94 0x65 0x05 0x81 0x53 0xe0 0x00 0x00 0xfb
 */