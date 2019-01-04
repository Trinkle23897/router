#include "lookuproute.h"
#define M 131063

Edge e[1<<20];
uint32_t last[32][M];
uint32_t et = 0, small = 32, large = 0;
/*
 * 参数 1 是目的地址,参数 2 是掩码长度,参数 3 是接口名,参数 4 是接口索引值,参数 5 是下一跳 IP 地址。
 */
int32_t insert_route(uint32_t ip4prefix, uint32_t prefixlen, char *ifname, uint32_t nexthopaddr)
{
	uint32_t ifindex = if_nametoindex(ifname);
	if (prefixlen < small) small = prefixlen;
	if (prefixlen > large) large = prefixlen;
	// printf("insert %d.%d.%d.%d/%d %s %d.%d.%d.%d\n", TOIP(ip4prefix), prefixlen, ifname, TOIP(nexthopaddr));
	// insert 192.168.6.0/24 eth14 192.168.3.2

	uint32_t ip = ntohl(ip4prefix) & (0xFFFFFFFF << (32 - prefixlen));
	uint32_t key = ip % M;
	for (uint32_t i = last[prefixlen][key]; i; i = e[i].nxt)
		if (e[i].ip == ip) {
			e[i] = (Edge){ip, nexthopaddr, ifindex, e[i].nxt, ifname};
			return 0;
		}
	e[++et] = (Edge){ip, nexthopaddr, ifindex, last[prefixlen][key], ifname};
	last[prefixlen][key] = et;
	return 0;
}

int32_t modify_route(uint32_t ip4prefix, uint32_t prefixlen, char* ifname, uint32_t nexthopaddr) {
	// RIP only
	// ip4prefix = ntohl(ip4prefix);
	// uint32_t ifindex = if_nametoindex(ifname);
	// for (int i = 0; i < route_table.size(); ++i) {
	// 	uint32_t tmpip = route_table[i].ip4prefix.s_addr;
	// 	uint32_t pre = route_table[i].prefixlen;
	// 	if (tmpip == ip4prefix && pre == prefixlen) {
	// 		route_table[i].nexthop->ifname = ifname;
	// 		route_table[i].nexthop->ifindex = ifindex;
	// 		route_table[i].nexthop->nexthopaddr.s_addr = nexthopaddr;
	// 		break;
	// 	}
	// }
}

/*
 * 参数1是目的IP地址，参数2下一跳和出接口信息
 * 注意：查找算法与所用路由表存储结构相关
 */
int32_t lookup_route(uint32_t dstaddr, nexthop *nexthopinfo)
{ // assume nexthopinfo != NULL
	uint32_t dstip = ntohl(dstaddr), target;
	for (uint32_t m = large; m >= small; --m) {
		target = dstip & (0xFFFFFFFF << (32 - m));
		for (uint32_t i = last[m][target % M]; i; i = e[i].nxt)
			if (e[i].ip == target) {
				nexthopinfo->ifname = e[i].ifname;
				nexthopinfo->ifindex = e[i].ifindex;
				nexthopinfo->nexthopaddr.s_addr = e[i].nexthop;
				return 0;
			}
	}
	return -1;
}

/*
 * 参数1是目的ip地址，参数2是掩码长度
 */
int32_t delete_route(uint32_t dstaddr, uint32_t prefixlen)
{
	// fprintf(stderr, "delete route: %d.%d.%d.%d/%d size %lu\n", TOIP(dstaddr), prefixlen, route_table.size());
	// // delete route: 192.168.3.2/24
	// for (std::vector<route>::iterator i = route_table.begin(); i != route_table.end();)
	// 	if ((*i).prefixlen == prefixlen && ((*i).nexthop)->nexthopaddr.s_addr == dstaddr)
	// 		i = route_table.erase(i);
	// 	else
	// 		++i;
}
