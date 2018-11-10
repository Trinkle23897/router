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
	// add-24 del-25
	while (1)
	{
        int32_t conn_fd = accept(sock_fd, (sockaddr*)NULL, NULL);
        int32_t ret = recv(conn_fd, &selfrt, sizeof(selfroute), 0);
		if (ret > 0)
		{
			if (selfrt.cmdnum == 24)
			{
				if_indextoname(selfrt.ifindex, ifname);
				insert_route(selfrt.prefix.s_addr, selfrt.prefixlen, ifname, selfrt.ifindex, selfrt.nexthop.s_addr);
			}
			else if (selfrt.cmdnum == 25)
				delete_route(selfrt.nexthop, selfrt.prefixlen);
		}

	}

}

void analyseIP(ip*iphd)
{
    static int32_t cnt = 0;
    uint32_t src = iphd->ip_src.s_addr;
    uint32_t dst = iphd->ip_dst.s_addr;
    fprintf(stderr, "%4d src: %d.%d.%d.%d    dst: %d.%d.%d.%d\n", ++cnt, TOIP(src), TOIP(dst));
}

int main(int argc, char** argv)
{
	char skbuf[1512], *data;
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

	// 路由表初始化
	// route_table = new route;
	// if (route_table == NULL)
	// 	return printf("malloc route error\n"), -1;
	// memset(route_table, 0, sizeof(route));

	// 创建线程去接收路由信息
	int32_t pd = pthread_create(&tid, NULL, thr_fn, NULL);

	while (1)
	{
		// 接收ip数据包模块
		recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
		if (recvlen > 0)
		{
			ip_recvpkt = (ip*)(skbuf + ETHER_HEADER_LEN);
			if (ip_recvpkt->ip_src.s_addr != ip_recvpkt->ip_dst.s_addr && ip_recvpkt->ip_dst.s_addr != inet_addr("255.255.255.255"))
			{
				// 分析打印ip数据包的源和目的ip地址
				analyseIP(ip_recvpkt);
				data = (char*)(skbuf + ETHER_IP_LEN);
				// 校验计算模块
				ip *iphead = (ip*)(skbuf + ETHER_HEADER_LEN);
				// 调用校验函数check_sum，成功返回1
				if (check_sum(iphead) == 1)
					fprintf(stderr, "checksum ok\n");
				else {
					fprintf(stderr, "checksum error\n");
					continue;
				}
				// 查找路由表，获取下一跳ip地址和出接口模块
				// 调用查找路由函数lookup_route，获取下一跳ip地址和出接口
				if (lookup_route(iphead->ip_dst, nexthopinfo) == 0)
				{
					//success
				}
				else {
					fprintf(stderr, "no next hop\n");
					continue;
				}
				// arp find
				//调用arpGet获取下一跳的mac地址
				if (arpGet(srcmac, nexthopinfo) == 0)
				{
					// success
				}
				else {
					fprintf(stderr, "arp find err\n");
					continue;
				}
                ip_transmit(iphead, );
				// 调用计算校验和函数count_check_sum，返回新的校验和
				iphead->ip_sum = count_check_sum(iphead, true);
				// send ether icmp
				// 调用ip_transmit函数   填充数据包，通过原始套接字从查表得到的出接口(比如网卡2)将数据包发送出去
				// 将获取到的下一跳接口信息存储到存储接口信息的结构体ifreq里，通过ioctl获取出接口的mac地址作为数据包的源mac地址
				// 封装数据包：
				// <1>.根据获取到的信息填充以太网数据包头，以太网包头主要需要源mac地址、目的mac地址、以太网类型eth_header->ether_type = htons(ETHERTYPE_IP);
				// <2>.再填充ip数据包头，对其进行校验处理；
				// <3>.然后再填充接收到的ip数据包剩余数据部分，然后通过raw socket发送出去
				
			}
		}
	}
	close(recvfd);
	return 0;
}
/*
查某个 interface 的 MAC 的方法:
struct ifreq ifr;
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
strncpy(ifr.ifr_name, "eth1", IF_NAMESIZE);
if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0) {
    memcpy(mac_a, ifr.ifr_hwaddr.sa_data, 6);
} else {
    // something really goes wrong here, doesn't it?
}
close(sockfd);
*/
/*
发包的方法:
首先确保 IP 头、以太网头都正确修改了。
int sendfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
struct sockaddr_ll sadr_ll;
sadr_ll.sll_ifindex = 0; // index of next hop
sadr_ll.sll_halen = ETH_ALEN;
// mac_addr_to is the result of arp query
memcpy(sadr_ll.sll_addr, mac_addr_to, ETH_ALEN);
// length should be equal to the length you receive from raw socket
if ((result = sendto(sendfd, skbuf, length, 0,
(const struct sockaddr *)&sadr_ll,sizeof(struct sockaddr_ll))) == -1) {
// send error
} else {
// send succeed
}
close(sendfd);
*/
