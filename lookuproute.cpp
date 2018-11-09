#include "lookuproute.h"

std::vector<route> route_table;

/*
 * 参数 1 是目的地址,参数 2 是掩码长度,参数 3 是接口名,参数 4 是接口索引值,参数 5 是下一跳 IP 地址。
 */
int32_t insert_route(uint32_t ip4prefix, uint32_t prefixlen, char *ifname, uint32_t ifindex, uint32_t nexthopaddr)
{
	route tmp;
	tmp.ip4prefix.s_addr = ip4prefix;
	tmp.prefixlen = prefixlen;
	tmp.nexthop = new nexthop;
	tmp.nexthop->ifname = ifname;
	tmp.nexthop->ifindex = ifindex;
	tmp.nexthop->nexthopaddr.s_addr = nexthopaddr;
	tmp.nexthop->next = NULL;
	route_table.push_back(tmp);
	return 0;
}

/*
 * 参数1是目的IP地址，参数2下一跳和出接口信息
 * 注意：查找算法与所用路由表存储结构相关
 */
int32_t lookup_route(in_addr dstaddr, nextaddr *nexthopinfo)
{
	fprintf(stderr, "lookup: %X\n", dstaddr.s_addr);
}

/*
 * 参数1是目的ip地址，参数2是掩码长度
 */
int32_t delete_route(in_addr dstaddr, uint32_t prefixlen)
{
	fprintf(stderr, "delete route: %X %d\n", dstaddr.s_addr, prefixlen);
}