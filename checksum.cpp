#include "checksum.h"

/*
 * 参数1是指向IP数据包头的指针ip_recvpkt，参数2是IP数据包的校验和
 */
int32_t check_sum(ip* iphd, uint16_t checksum)
{
	uint16_t csum = count_check_sum(iphd, false);
	return csum == iphd->ip_sum ? 0 : -1;
}

/*
 * 参数是指向IP数据包头的指针ip_recvpkt
 * 注意：重新计算报文校验和前，ttl值减1
 */
uint16_t count_check_sum(ip* iphd, bool decline)
{
	uint16_t oldsum = iphd->ip_sum;
	iphd->ip_sum = 0;
	if (decline)
		iphd->ip_ttl -= 1;
	uint16_t* buf = (uint16_t*) iphd;
	uint32_t sum = 0;
	for (uint16_t len = 10; len > 0; --len)
		sum += *buf++;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += sum >> 16;
	iphd->ip_sum = oldsum;
	return ~sum;
}