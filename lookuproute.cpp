#include "lookuproute.h"

route *route_table;

/*
 * 参数 1 是目的地址,参数 2 是掩码长度,参数 3 是接口名,参数 4 是接口索引值,参数 5 是下一跳 IP 地址。
 */
int32_t insert_route(uint32_t ip4prefix, uint32_t prefixlen, char *ifname, uint32_t ifindex, uint32_t nexthopaddr)
{

}

/*
 * 参数1是目的IP地址，参数2下一跳和出接口信息
 * 注意：查找算法与所用路由表存储结构相关
 */
int32_t lookup_route(in_addr dstaddr, struct nextaddr *nexthopinfo)
{

}

/*
 * 参数1是目的ip地址，参数2是掩码长度
 */
int32_t delete_route(in_addr dstaddr, uint32_t prefixlen)
{

}