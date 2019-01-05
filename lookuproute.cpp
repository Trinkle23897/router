#include "lookuproute.h"
#define M 3277

Edge e[1<<15], lut[13][1<<13];
uint32_t last[9][M];
uint32_t et = 0;
uint32_t mask_flag7 = 0, mask_flag12 = 0;
/*
 * 参数 1 是目的地址,参数 2 是掩码长度,参数 3 是接口名,参数 4 是接口索引值,参数 5 是下一跳 IP 地址。
 */
int32_t insert_route(uint32_t trueip, uint32_t mask, char *ifname, uint32_t next)
{
	uint32_t ifindex = if_nametoindex(ifname);
	// printf("insert %d.%d.%d.%d/%d %s %d.%d.%d.%d\n", TOIP(trueip), mask, ifname, TOIP(next));
	// insert 192.168.6.0/24 eth14 192.168.3.2
	if (mask > 12) {
		uint32_t m0 = (((mask - 1) >> 2) << 2) + 1;
		uint32_t ip = ntohl(trueip) >> 32 - m0;
		trueip = ntohl(trueip) >> 32 - mask;
		uint32_t key = ip % M;
		bool flag = false;
		for (uint32_t i = last[mask - 1 >> 2][key]; i; i = e[i].nxt)
			if (e[i].ip == trueip && e[i].mask == mask) {
				e[i] = (Edge){e[i].nxt, trueip, mask, ifname, ifindex, next};
				flag = true;
			}
		if (!flag) {
			e[++et] = (Edge){last[mask - 1 >> 2][key], trueip, mask, ifname, ifindex, next};
			last[mask - 1 >> 2][key] = et;
		}
	} else {
		if (mask <= 7)
			mask_flag7 = 1;
		if (mask <= 12)
			mask_flag12 = 1;
		trueip = ntohl(trueip) >> 32 - mask;
		// for (int i = mask; i <= 8; ++i) mask_flag[i] = 1;
		lut[mask][trueip] = (Edge){1, trueip, mask, ifname, ifindex, next};
	}
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
int32_t lookup_route(uint32_t d0, nexthop *nexthopinfo)
{ // assume nexthopinfo != NULL
	d0 = ntohl(d0);
	uint32_t max_mask = 0, id;
#define foredge(m0) \
	if (last[m0 - 1 >> 2][(d0 >> 35 - m0) % M]) {\
		for (uint32_t i = last[m0 - 1 >> 2][(d0 >> 35 - m0) % M]; i; i = e[i].nxt) {\
			if (e[i].mask == m0 && max_mask < m0 && e[i].ip == (d0 >> 32 - m0)) {\
				max_mask = m0;\
				nexthopinfo->ifname = e[i].ifname;\
				nexthopinfo->ifindex = e[i].ifindex;\
				nexthopinfo->nexthopaddr.s_addr = e[i].nexthop;\
				return 0;\
			} else if (e[i].mask == m0 - 1 && max_mask < m0 - 1 && e[i].ip == (d0 >> 33 - m0)) {\
				max_mask = m0 - 1;\
				id = i;\
			} else if (e[i].mask == m0 - 2 && max_mask < m0 - 2 && e[i].ip == (d0 >> 34 - m0)) {\
				max_mask = m0 - 2;\
				id = i;\
			} else if (e[i].mask == m0 - 3 && max_mask < m0 - 3 && e[i].ip == (d0 >> 35 - m0)) {\
				max_mask = m0 - 3;\
				id = i;\
			}\
		}\
		if (max_mask > 0) {\
			nexthopinfo->ifname = e[id].ifname;\
			nexthopinfo->ifindex = e[id].ifindex;\
			nexthopinfo->nexthopaddr.s_addr = e[id].nexthop;\
			return 0;\
		}\
	}
	foredge(32) foredge(28) foredge(24) foredge(20) foredge(16)
#define forlut(m0)\
	if (lut[m0][d0 >> 32 - m0].nxt) {\
		nexthopinfo->ifname = lut[m0][d0 >> 32 - m0].ifname;\
		nexthopinfo->ifindex = lut[m0][d0 >> 32 - m0].ifindex;\
		nexthopinfo->nexthopaddr.s_addr = lut[m0][d0 >> 32 - m0].nexthop;\
		return 0;\
	}
	if (!mask_flag12) return -1;
	forlut(12) forlut(11) forlut(10) forlut(9) forlut(8)
	if (!mask_flag7) return -1;
	forlut(7) forlut(6) forlut(5)
	forlut(4) forlut(3) forlut(2) forlut(1)
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
