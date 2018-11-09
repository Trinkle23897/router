// install g++: https://my.oschina.net/liuyes/blog/1609644
#include "checksum.h"
#include "lookuproute.h"
#include "arpfind.h"
#include "sendetherip.h"
#include "recvroute.h"

//接收路由信息的线程
void *thr_fn(void *arg)
{
	selfroute selfrt;
	//get if.name
	struct if_nameindex* ifni = if_nameindex();
	struct if_nameindex* head = ifni;
	char *ifname;

	// add-24 del-25
	while (1)
	{
		if (static_route_get(&selfrt) == 1)
		{
			if (selfrt.cmdnum == 24)
			{
				if_indextoname(selfrt.ifindex, ifname);
				//插入到路由表里
				insert_route(selfrt.prefix.s_addr, selfrt.prefixlen, ifname, selfrt.ifindex, selfrt.nexthop.s_addr);
				// insert_route(inet_addr("192.168.6.0"), 24, "eth14", if_nametoindex("eth14"), inet_addr("192.168.3.2"));
			}
			else if (selfrt.cmdnum == 25) //从路由表里删除路由
				delete_route(selfrt.nexthop, selfrt.prefixlen);
		}

	}

}

int main(int argc, char** argv)
{
	char skbuf[1512], *data;
	int32_t recvfd, datalen, recvlen;
	pthread_t tid;
	ip *ip_recvpkt;
	nextaddr *nexthopinfo = new nextaddr;
	arpmac *srcmac = new arpmac;

	// 创建raw socket套接字
	if ((recvfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) == -1)
	{
		printf("recvfd() error\n");
		return -1;
	}

	// 路由表初始化
	// route_table = new route;
	// if (route_table == NULL)
	// 	return printf("malloc route error\n"), -1;
	// memset(route_table, 0, sizeof(route));

	// 调用添加函数insert_route往路由表里添加直连路由 for test

	// 创建线程去接收路由信息
	int32_t pd = pthread_create(&tid, NULL, thr_fn, NULL);

	while (1)
	{
		// 接收ip数据包模块
		recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
		if (recvlen > 0)
		{
			ip_recvpkt = (ip*)(skbuf + ETHER_HEADER_LEN);
			if (ip_recvpkt->ip_src.s_addr != ip_recvpkt->ip_dst.s_addr)
			{
				// 分析打印ip数据包的源和目的ip地址
				//	analyseIP(ip_recvpkt);
				data = (char*)(skbuf + ETHER_IP_LEN);
				// 校验计算模块
				ip *iphead = (ip*)(skbuf + ETHER_HEADER_LEN);
				// 调用校验函数check_sum，成功返回1
				if (check_sum(iphead) == 1)
					printf("checksum ok\n");
				else {
					printf("checksum error\n");
					continue;
				}
				// 调用计算校验和函数count_check_sum，返回新的校验和
				iphead->ip_sum = count_check_sum(iphead, true);
				// 查找路由表，获取下一跳ip地址和出接口模块
				// 调用查找路由函数lookup_route，获取下一跳ip地址和出接口
				if (lookup_route(iphead->ip_dst, nexthopinfo) == 0)
				{
					//success
				}
				else {
					printf("no next hop\n");
					continue;
				}
				// arp find
				//调用arpGet获取下一跳的mac地址
				if (arpGet(srcmac, nexthopinfo) == 0)
				{
					// success
				}
				else {
					printf("arp find err\n");
					continue;
				}
				// send ether icmp
				{

					// 调用ip_transmit函数   填充数据包，通过原始套接字从查表得到的出接口(比如网卡2)将数据包发送出去
					// 将获取到的下一跳接口信息存储到存储接口信息的结构体ifreq里，通过ioctl获取出接口的mac地址作为数据包的源mac地址
					// 封装数据包：
					// <1>.根据获取到的信息填充以太网数据包头，以太网包头主要需要源mac地址、目的mac地址、以太网类型eth_header->ether_type = htons(ETHERTYPE_IP);
					// <2>.再填充ip数据包头，对其进行校验处理；
					// <3>.然后再填充接收到的ip数据包剩余数据部分，然后通过raw socket发送出去
				}
			}
		}
	}
	close(recvfd);
	return 0;
}