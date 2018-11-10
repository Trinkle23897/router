#ifndef __DEFINES__
#define __DEFINES__ 

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <netdb.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netpacket/packet.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/* 以太网帧首部长度 */
#define ETHER_HEADER_LEN sizeof(ether_header)
/* 整个ip首部长度 */
#define IP_HEADER_LEN sizeof(ip)
/* 以太网 + 整个ip首部长度*/
#define ETHER_IP_LEN ETHER_HEADER_LEN + IP_HEADER_LEN
/* IP地址长度 */
#define IP_ADDR_LEN 4

#define TOIP(x) (x)%256, ((x)>>8)%256, ((x)>>16)%256, ((x>>24))%256
#define TOMAC(x) (x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5]

#endif
