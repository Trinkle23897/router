#include "checksum.h"
#include "lookuproute.h"
#include "arpfind.h"
#include "recvroute.h"
#include "rip.h"

//接收路由信息的线程
void *thr_fn(void *arg)
{
	selfroute selfrt;
	char *ifname = new char[IF_NAMESIZE];
	int32_t sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in server_addr;
	bzero(&server_addr, sizeof(sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(800);
	if (bind(sock_fd, (sockaddr*)(&server_addr), sizeof(sockaddr))) {
		fprintf(stderr, "cannot bind socket 800\n");
		return NULL;
	}
	listen(sock_fd, 5);
	while (1)
	{
		int32_t conn_fd = accept(sock_fd, (sockaddr*)NULL, NULL);
		int32_t ret = recv(conn_fd, &selfrt, sizeof(selfroute), 0);
		if (ret > 0)
		{
			if (selfrt.cmdnum == AddRoute)
			{
				if_indextoname(selfrt.ifindex, ifname);
				insert_route(selfrt.prefix.s_addr, selfrt.prefixlen, ifname, selfrt.nexthop.s_addr);
			}
			else if (selfrt.cmdnum == DelRoute)
				delete_route(selfrt.nexthop.s_addr, selfrt.prefixlen);
		}
	}
}

void analyseIP(ip*iphd)
{
	return;
	static int32_t cnt = 0;
	uint32_t src = iphd->ip_src.s_addr;
	uint32_t dst = iphd->ip_dst.s_addr;
	fprintf(stderr, "%4d src: %d.%d.%d.%d    dst: %d.%d.%d.%d\n", ++cnt, TOIP(src), TOIP(dst));
}

int main(int argc, char** argv)
{
	const uint32_t SOCKET_LEN = 1512;
	char skbuf[SOCKET_LEN], *data;
	int32_t recvfd, datalen, recvlen;
	pthread_t tid;
	ip *ip_recvpkt;
	nexthop *nexthopinfo = new nexthop;
	arpmac *srcmac = new arpmac;

	// 创建raw socket套接字
	if ((recvfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) == -1)
	{
		fprintf(stderr, "recvfd() error\n");
		return -1;
	}

	// 创建线程去接收路由信息
	int32_t pd = pthread_create(&tid, NULL, thr_fn, NULL);
	start_rip();
	puts("OK");
	while (1)
	{
		// 接收ip数据包模块
		recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
		if (recvlen > 0)
		{
			ip_recvpkt = (ip*)(skbuf + ETHER_HEADER_LEN);
			ether_header* eh = (ether_header*)skbuf;
			// fprintf(stderr, "dhost: %x:%x:%x:%x:%x:%x\n", TOMAC(eh->ether_dhost));
			// fprintf(stderr, "shost: %x:%x:%x:%x:%x:%x\n", TOMAC(eh->ether_shost));
			// fprintf(stderr, "ether_type %d\n", eh->ether_type);
			if (ip_recvpkt->ip_src.s_addr != ip_recvpkt->ip_dst.s_addr && ip_recvpkt->ip_dst.s_addr != inet_addr("255.255.255.255"))
			{
				// 分析打印ip数据包的源和目的ip地址
				// analyseIP(ip_recvpkt);
				data = (char*)(skbuf + ETHER_IP_LEN);
				// 校验计算模块
				ip *iphead = (ip*)(skbuf + ETHER_HEADER_LEN);
				// 调用校验函数check_sum，成功返回1
				if (check_sum(iphead) == 1)
					;//fprintf(stderr, "checksum ok\n");
				else {
					fprintf(stderr, "checksum error\n");
					continue;
				}
				// 查找路由表，获取下一跳ip地址和出接口模块
				// 调用查找路由函数lookup_route，获取下一跳ip地址和出接口
				if (lookup_route(iphead->ip_dst.s_addr, nexthopinfo) == 0)
				{
					// fprintf(stderr, "source %d.%d.%d.%d find next hop %d.%d.%d.%d\n", TOIP(iphead->ip_dst.s_addr), TOIP(nexthopinfo->nexthopaddr.s_addr));
				}
				else {
					fprintf(stderr, "no next hop\n");
					continue;
				}
				// arp find
				//调用arpGet获取下一跳的mac地址
				if (nexthopinfo->nexthopaddr.s_addr == 0)
					nexthopinfo->nexthopaddr.s_addr = ip_recvpkt->ip_dst.s_addr;
				if (arpGet(srcmac, nexthopinfo) == 0)
				{
					;//fprintf(stderr, "get next hop arp\n");
				}
				else {
					fprintf(stderr, "arp find err\n");
					continue;
				}
				struct ifreq ifr;
				int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
				strncpy(ifr.ifr_name, nexthopinfo->ifname, IF_NAMESIZE);
				if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0) {
					memcpy(eh->ether_dhost, srcmac->mac, ETH_ALEN);
					memcpy(eh->ether_shost, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
					eh->ether_type = htons(ETHERTYPE_IP);
				} else {
					fprintf(stderr, "could not find %s mac addr\n", nexthopinfo->ifname);
				}
				close(sockfd);
				iphead->ip_sum = count_check_sum(iphead, true);
				// 调用计算校验和函数count_check_sum，返回新的校验和
				// send ether icmp
				// 调用ip_transmit函数   填充数据包，通过原始套接字从查表得到的出接口(比如网卡2)将数据包发送出去
				// 将获取到的下一跳接口信息存储到存储接口信息的结构体ifreq里，通过ioctl获取出接口的mac地址作为数据包的源mac地址
				// 封装数据包：
				// <1>.根据获取到的信息填充以太网数据包头，以太网包头主要需要源mac地址、目的mac地址、以太网类型eth_header->ether_type = htons(ETHERTYPE_IP);
				// <2>.再填充ip数据包头，对其进行校验处理；
				// <3>.然后再填充接收到的ip数据包剩余数据部分，然后通过raw socket发送出去
				int sendfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
				sockaddr_ll sadr_ll;
				sadr_ll.sll_ifindex = nexthopinfo->ifindex;
				sadr_ll.sll_halen = ETH_ALEN;
				// mac_addr_to is the result of arp query
				memcpy(sadr_ll.sll_addr, srcmac->mac, ETH_ALEN);
				// length should be equal to the length you receive from raw socket
				if (sendto(sendfd, skbuf, SOCKET_LEN, 0,
					(const sockaddr*)&sadr_ll, sizeof(sockaddr_ll)) == -1) {
					fprintf(stderr, "send error\n");
				} else
					;//fprintf(stderr, "send succeed\n");
				close(sendfd);
			}
		}
	}
	close(recvfd);
	return 0;
}
