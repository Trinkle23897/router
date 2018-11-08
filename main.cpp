#include "checksum.h"
#include "lookuproute.h"
#include "arpfind.h"
#include "sendetherip.h"
#include "recvroute.h"

//接收路由信息的线程
void *thr_fn(void *arg)
{
	int st = 0;
	struct selfroute *selfrt;
	selfrt = (struct selfroute*)malloc(sizeof(struct selfroute));
	memset(selfrt, 0, sizeof(struct selfroute));

	//get if.name
	struct if_nameindex *head, *ifni;
	ifni = if_nameindex();
	head = ifni;
	char *ifname;

	// add-24 del-25
	while (1)
	{
		st = static_route_get(selfrt);
		if (st == 1)
		{
			if (selfrt->cmdnum == 24)
			{
				while (ifni->if_index != 0) {
					if (ifni->if_index == selfrt->ifindex)
					{
						printf("if_name is %s\n", ifni->if_name);
						ifname = ifni->if_name;
						break;
					}
					ifni++;
				}

				{
					//插入到路由表里
				}
			}
			else if (selfrt->cmdnum == 25)
			{
				//从路由表里删除路由
			}
		}

	}

}

int main()
{
	char skbuf[1512];
	char data[1480];
	int recvfd, datalen;
	int recvlen;
	struct ip *ip_recvpkt;
	pthread_t tid;
	ip_recvpkt = (struct ip*)malloc(sizeof(struct ip));

	//创建raw socket套接字
	if ((recvfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) == -1)
	{
		printf("recvfd() error\n");
		return -1;
	}

	//路由表初始化
	route_table = (struct route*)malloc(sizeof(struct route));

	if (route_table == NULL)
	{
		printf("malloc error!!\n");
		return -1;
	}
	memset(route_table, 0, sizeof(struct route));


	{

		//调用添加函数insert_route往路由表里添加直连路由

	}

	//创建线程去接收路由信息
	int pd;
	pd = pthread_create(&tid, NULL, thr_fn, NULL);


	while (1)
	{
		//接收ip数据包模块
		recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
		if (recvlen > 0)
		{

			ip_recvpkt = (struct ip *)(skbuf + ETHER_HEADER_LEN);

			//192.168.1.10是测试服务器的IP，现在测试服务器IP是192.168.1.10到192.168.1.80.
			//使用不同的测试服务器要进行修改对应的IP。然后再编译。
			//192.168.6.2是测试时候ping的目的地址。与静态路由相对应。
			if (ip_recvpkt->ip_src.s_addr == inet_addr("192.168.1.10") && ip_recvpkt->ip_dst.s_addr == inet_addr("192.168.6.2") )
			{
				//分析打印ip数据包的源和目的ip地址
				//	analyseIP(ip_recvpkt);

				int s;
				for (s = 0; s < 1480; s++)
				{
					data[s] = skbuf[s + 34];
				}


				// 校验计算模块
				struct ip *iphead;
				int c = 0;

				iphead = (struct ip *)malloc(sizeof(struct ip));

				{

					//调用校验函数check_sum，成功返回1
				}
				if (c == 1)
				{
					printf("checksum is ok!!\n");
				} else
				{
					printf("checksum is error !!\n");
					return -1;
				}

				{

					//调用计算校验和函数count_check_sum，返回新的校验和
				}


				//查找路由表，获取下一跳ip地址和出接口模块
				struct nextaddr *nexthopinfo;
				nexthopinfo = (struct nextaddr *)malloc(sizeof(struct nextaddr));
				memset(nexthopinfo, 0, sizeof(struct nextaddr));
				{

					//调用查找路由函数lookup_route，获取下一跳ip地址和出接口
				}


				//arp find
				struct arpmac *srcmac;
				srcmac = (struct arpmac*)malloc(sizeof(struct arpmac));
				memset(srcmac, 0, sizeof(struct arpmac));
				{

					//调用arpGet获取下一跳的mac地址
				}

				//send ether icmp
				{
					//调用ip_transmit函数   填充数据包，通过原始套接字从查表得到的出接口(比如网卡2)将数据包发送出去
					//将获取到的下一跳接口信息存储到存储接口信息的结构体ifreq里，通过ioctl获取出接口的mac地址作为数据包的源mac地址
					//封装数据包：
					//<1>.根据获取到的信息填充以太网数据包头，以太网包头主要需要源mac地址、目的mac地址、以太网类型eth_header->ether_type = htons(ETHERTYPE_IP);
					//<2>.再填充ip数据包头，对其进行校验处理；
					//<3>.然后再填充接收到的ip数据包剩余数据部分，然后通过raw socket发送出去
				}
			}



		}
	}

	close(recvfd);
	return 0;
}