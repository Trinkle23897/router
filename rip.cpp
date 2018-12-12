#include "rip.h"

std::vector<TRtEntry> rip_table;
std::vector<Interface> iface; // 存储本地接口

void rip_sendpkt(uint8_t *data, uint32_t len, uint32_t addr, uint16_t port = RIP_PORT) {
	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		perror("Opening datagram socket error when send\n");
		exit(1);
	} else 
		printf("Opening datagram socket....OK.\n");

	// 防止绑定地址冲突，仅供参考
	// 设置地址重用
	int iReUseddr = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&iReUseddr, sizeof(iReUseddr)) < 0) {
		perror("setsockopt reuseaddr error\n");
		return;
	}
	// 设置端口重用
	int iReUsePort = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&iReUsePort, sizeof(iReUsePort)) < 0){
		perror("setsockopt reuseport error\n");
		return;
	}

	// 防止组播回环的参考代码	
	// 0 禁止回环  1开启回环
	int loop = 0;
	int err = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
	if (err < 0) {
		perror("setsockopt():IP_MULTICAST_LOOP\n");
	}

	// sockaddr_in* localSock = new sockaddr_in();
	// localSock->sin_family = AF_INET;
	// localSock->sin_port = htons(port);
	// localSock->sin_addr.s_addr = INADDR_ANY;

	// if (bind(fd, (sockaddr*)localSock, sizeof(sockaddr_in))) {
	// 	perror("Binding datagram socket error\n");
	// 	close(fd);
	// 	exit(1);
	// } else printf("Binding datagram socket...OK.\n");
	
	in_addr localAddr;
	localAddr.s_addr = addr;
	printf("interface addr %d.%d.%d.%d\n", TOIP(localAddr.s_addr));
	// 绑定socket
	if(setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &localAddr, sizeof(localAddr)) < 0) {
		perror("Setting local interface error");
	} else printf("Setting the local interface...OK\n");

	sockaddr_in router;
	router.sin_family = AF_INET;
	router.sin_port = htons(port);
	router.sin_addr.s_addr = inet_addr(RIP_GROUP);
	int result;
	if ((result = sendto(fd, data, len, 0, (sockaddr*)&router, sizeof(sockaddr_in))) == -1) {
		printf("send error!\n");
	} else {
		printf("send succeed! packet len: %d\n", result);
	}
	close(fd);
}

void rip_multicast(uint32_t addr)
{
	// 封装请求包  command = 1, version = 2, family = 0, metric = 16
	TRipPkt sendpkt;
	sendpkt.ucCommand = RIP_RESPONSE;
	sendpkt.ucVersion = RIP_VERSION;
	sendpkt.usZero = 0;
	int index = 0;
	for (int i = 0; i < rip_table.size(); ++i) {
		if (i > 0 && index == RIP_MAX_ENTRY) {
			index = 0;
			rip_sendpkt((uint8_t*)(&sendpkt), sizeof(TRipPkt), addr);
		}
		if (rip_table[i].stIpPrefix.s_addr == addr) continue;
		sendpkt.RipEntries[index].stAddr = rip_table[i].stIpPrefix;
		sendpkt.RipEntries[index].stNexthop = rip_table[i].stNexthop;
		sendpkt.RipEntries[index].stPrefixLen.s_addr = (0xffffffff >> (32 - rip_table[i].uiPrefixLen));
		sendpkt.RipEntries[index].uiMetric = htonl(RIP_INFINITY);
		sendpkt.RipEntries[index].usFamily = 0;
		printf("--- %d.%d.%d.%d %d.%d.%d.%d\n", TOIP(rip_table[i].stIpPrefix.s_addr), TOIP(sendpkt.RipEntries[index].stPrefixLen.s_addr));
		++index;
	}
	if (index > 0)
		rip_sendpkt((uint8_t*)(&sendpkt), sizeof(TRipEntry) * index + 4, addr);
}

void* rip_recvpkt(void* args)
{
	// 接收ip设置
	int sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) perror("Opening datagram socket error when receiving\n");
	else printf("Opening datagram socket....OK.\n");
	// 防止绑定地址冲突
	// 设置地址重用
	int iReUseddr = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&iReUseddr, sizeof(iReUseddr)) < 0) {
		perror("setsockopt reuseaddr error\n");
		return NULL;
	}
	// 设置端口重用
	int iReUsePort = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, (const char*)&iReUsePort, sizeof(iReUsePort)) < 0) {
		perror("setsockopt reuseport error\n");
		return NULL;
	}
	// 把本地地址加入到组播
	sockaddr_in* localSock = new sockaddr_in();
	socklen_t sendsize = sizeof(sockaddr_in);
	localSock->sin_family = AF_INET;
	localSock->sin_port = htons(RIP_PORT);
	localSock->sin_addr.s_addr = INADDR_ANY;
	if (bind(sd, (sockaddr*)localSock, sizeof(sockaddr_in))) {
		perror("Binding datagram socket error\n");
		close(sd);
		return NULL;
	} else printf("Binding datagram socket...OK.\n");
	
	ip_mreq_source mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(RIP_GROUP);
	for (int i = 0; i < iface.size(); ++i) {
		mreq.imr_interface.s_addr = iface[i].addr;
		mreq.imr_sourceaddr.s_addr = iface[i].addr;
		if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(ip_mreq)) < 0) {
			perror("join multicast group failed\n");
			return NULL;
		}
		if (setsockopt(sd, IPPROTO_IP, IP_BLOCK_SOURCE, &mreq, sizeof(mreq)) < 0) {
			perror("block multicast from local interface failed\n");
			return NULL;
		}
	}

	int cnt = 0;
	uint8_t buf[sizeof(TRipPkt)];
	TRipPkt* recvpkt = (TRipPkt*)buf;
	while(1) {
		// 接收rip报文   存储接收源ip地址
		// 判断command类型，request 或 response
		int ret = recvfrom(sd, buf, sizeof(TRipPkt), 0, (sockaddr*)localSock, &sendsize);
		if (ret > 0) {
			printf("ret %d\tcnt: %d\tucCammand: %d\tucVersion: %d\n", ret, ++cnt, recvpkt->ucCommand, recvpkt->ucVersion);
		}
	}
}

void rip_timeout() {
	for (int i = 0; i < iface.size(); ++i)
		rip_multicast(iface[i].addr);
	puts("30s!");
}

void* count_30s(void* args) {
	while (1) {
		rip_timeout();
		sleep(30);
	}
}

// 将本地接口表添加到rip路由表里
void get_local_info()
{
	uint32_t localhost = inet_addr("127.0.0.1");
	ifaddrs *if_addr = NULL;	
	getifaddrs(&if_addr); // linux系统函数
	ifaddrs* head = if_addr;
	
	while (if_addr != NULL)
	{
		if (if_addr->ifa_addr->sa_family == AF_INET)
		{
			uint32_t addr = ((sockaddr_in*)if_addr->ifa_addr)->sin_addr.s_addr;
			printf("get %x\n", htonl(addr));
			if (addr != localhost)
			{
				// add interface
				Interface newface;
				newface.addr = addr;
				newface.name = new char[IF_NAMESIZE];
				strncpy(newface.name, if_addr->ifa_name, strlen(if_addr->ifa_name));
				iface.push_back(newface);
				// add to rip table
				TRtEntry newroute;
				newroute.stIpPrefix.s_addr = newface.addr;
				newroute.uiPrefixLen = 24;
				newroute.stNexthop.s_addr = 0;
				newroute.uiMetric = 0;
				newroute.pcIfname = newface.name;
				rip_table.push_back(newroute);
			}	
		}
		if_addr = if_addr->ifa_next;
	}
	freeifaddrs(head); // linux系统函数
}

void start_rip()
{
	get_local_info();
	// 创建更新线程，30s更新一次,向组播地址更新Update包
	pthread_t p0, p1;
	//int32_t pd0 = pthread_create(&p0, NULL, rip_recvpkt, NULL);
	//count_30s(NULL);
	int32_t pd1 = pthread_create(&p1, NULL, count_30s, NULL);
	rip_recvpkt(NULL);
}

int main(int argc, char* argv[])
{
	start_rip();
	return 0;
}
