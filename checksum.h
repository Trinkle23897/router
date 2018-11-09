#ifndef __CHECK__
#define __CHECK__

#include "defines.h"

// struct _iphdr //定义IP首部
// {
//     unsigned char h_verlen; //4位首部长度+4位IP版本号
//     unsigned char tos; //8位服务类型TOS
//     uint16_t total_len; //16位总长度（字节）
//     uint16_t ident; //16位标识
//     uint16_t frag; //16位标志位
//     unsigned char ttl; //8位生存时间 TTL
//     unsigned char proto; //8位协议 (TCP, UDP 或其他)
//     uint16_t checksum; //16位IP首部校验和
//     unsigned int sourceIP; //32位源IP地址
//     unsigned int destIP; //32位目的IP地址
// };

// /*
//  * Structure of an internet header, naked of options.
//  */
// struct ip
//   {
// #if __BYTE_ORDER == __LITTLE_ENDIAN
//     unsigned int ip_hl:4;       /* header length */
//     unsigned int ip_v:4;        /* version */
// #endif
// #if __BYTE_ORDER == __BIG_ENDIAN
//     unsigned int ip_v:4;        /* version */
//     unsigned int ip_hl:4;       /* header length */
// #endif
//     uint8_t ip_tos;         /* type of service */
//     unsigned short ip_len;      /* total length */
//     unsigned short ip_id;       /* identification */
//     unsigned short ip_off;      /* fragment offset field */
//     uint8_t ip_ttl;         /* time to live */
//     uint8_t ip_p;           /* protocol */
//     unsigned short ip_sum;      /* checksum */
//     struct in_addr ip_src, ip_dst;  /* source and dest address */
//   };  

int32_t check_sum(ip* iphd);
uint16_t count_check_sum(ip* iphd, bool decline);

#endif