#include "checksum.h"

/*
 * 参数1是指向IP数据包头的指针ip_recvpkt，参数2是IP数据包的校验和
 */
int32_t check_sum(uint16_t *iphd, uint16_t checksum)
{

}

/*
 * 参数是指向IP数据包头的指针ip_recvpkt 
 * 注意：重新计算报文校验和前，ttl值减1
 */
uint16_t count_check_sum(uint16_t *iphd)
{

}