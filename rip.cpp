#include "rip.h"

std::vector<TRtEntry> rip_table;
std::vector<Interface> iface; // 存储本地接口

#ifndef RIP_MAIN
extern int32_t insert_route(uint32_t, uint32_t, char*, uint32_t);
extern int32_t modify_route(uint32_t, uint32_t, char*, uint32_t);
extern int32_t delete_route(uint32_t, uint32_t);
#endif

void rip_sendpkt(uint8_t *data, uint32_t len, uint32_t addr, uint16_t port=RIP_PORT) {
	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		perror("Opening datagram socket error when send\n");
		exit(1);
	} else 
		;//printf("Opening datagram socket....OK.\n");

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
		perror("setsockopt IP_MULTICAST_LOOP error\n");
	}

	sockaddr_in* localSock = new sockaddr_in();
	localSock->sin_family = AF_INET;
	localSock->sin_port = htons(port);
	localSock->sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (sockaddr*)localSock, sizeof(sockaddr_in))) {
		perror("Binding datagram socket error\n");
		close(fd);
		exit(1);
	} else ;//printf("Binding datagram socket...OK.\n");
	
	in_addr localAddr;
	localAddr.s_addr = addr;
	// printf("interface addr %d.%d.%d.%d\n", TOIP(localAddr.s_addr));
	// 绑定socket
	if(setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &localAddr, sizeof(localAddr)) < 0) {
		perror("Setting local interface error");
	} else ;//printf("Setting the local interface...OK\n");

	sockaddr_in router;
	router.sin_family = AF_INET;
	router.sin_port = htons(port);
	router.sin_addr.s_addr = inet_addr(RIP_GROUP);
	int result;
	if ((result = sendto(fd, data, len, 0, (sockaddr*)&router, sizeof(sockaddr_in))) == -1) {
		printf("send error!\n");
	} else {
		printf("send to %3d.%3d.%3d.%3d succeed! packet len: %d\n", TOIP(addr), result);
	}
	close(fd);
}

void rip_timeout_handler(uint32_t addr) {
	TRipPkt sendpkt;
	sendpkt.cmd = RIP_RESPONSE;
	sendpkt.ver = RIP_VERSION;
	sendpkt.zero = 0;
	int index = 0;
	for (int i = 0; i < rip_table.size(); ++i) {
		if (i > 0 && index == RIP_MAX_ENTRY) {
			index = 0;
			rip_sendpkt((uint8_t*)(&sendpkt), sizeof(TRipPkt), addr);
		}
		if (rip_table[i].addr.s_addr == addr) continue;
		fillEntry(sendpkt.entries[index], htons(2), 0, rip_table[i].addr.s_addr & rip_table[i].mask.s_addr, rip_table[i].mask, rip_table[i].nexthop, rip_table[i].metric);
		++index;
	}
	if (index > 0)
		rip_sendpkt((uint8_t*)(&sendpkt), sizeof(TRipEntry) * index + RIP_PACKET_HEAD, addr);
}

Interface* lookfor24(uint32_t addr) {
	uint32_t mask = 0xffffffff >> 8;
	for (int i = 0; i < iface.size(); ++i)
		if ((iface[i].addr & mask) == (addr & mask))
			return &(iface[i]);
	return NULL;
}

void rip_recv_handler(TRipPkt* rip_in, int32_t len, uint32_t from_addr) {
	if (lookfor24(from_addr) == NULL) return;
	uint16_t cmd = rip_in->cmd;
	uint16_t ver = rip_in->ver;
	if (cmd != RIP_REQUEST && cmd != RIP_RESPONSE) return;
	if (ver != RIP_VERSION) return;
	TRipPkt rip_out;
	rip_out.ver = RIP_VERSION;
	rip_out.zero = 0;
	if (cmd == RIP_REQUEST) {
		rip_out.cmd = RIP_RESPONSE;
		int index = 0;
		for (int i = 0; i < rip_table.size(); ++i) {
			if (i > 0 && index == RIP_MAX_ENTRY) {
				index = 0;
				rip_sendpkt((uint8_t*)(&rip_out), sizeof(TRipPkt), from_addr);
			}
			if (rip_table[i].addr.s_addr == from_addr) continue;
			fillEntry(rip_out.entries[index], htons(2), 0, rip_table[i].addr.s_addr & rip_table[i].mask.s_addr, rip_table[i].mask, rip_table[i].nexthop, rip_table[i].metric);
			++index;
		}
		if (index > 0)
			rip_sendpkt((uint8_t*)(&rip_out), sizeof(TRipEntry) * index + RIP_PACKET_HEAD, lookfor24(from_addr)->addr);
	} else { // response
		len = (len - RIP_PACKET_HEAD) / sizeof(TRipEntry);
		for (int k = 0; k < len; ++k) {
			bool appear = false;
			for (int i = 0; i < rip_table.size(); ++i) {
				if ((rip_table[i].addr.s_addr & rip_table[i].mask.s_addr) == (rip_in->entries[k].addr.s_addr & rip_in->entries[k].mask.s_addr)) {
					appear = true;
					uint32_t in_dist = htonl(rip_in->entries[k].metric) + 1;
					if (rip_table[i].nexthop.s_addr == from_addr) {
						rip_table[i].metric = min(in_dist, RIP_INFINITY);
#ifndef RIP_MAIN
						if (rip_table[i].metric >= RIP_INFINITY)
							delete_route(rip_table[i].nexthop.s_addr, __builtin_ctz(~rip_table[i].mask.s_addr));
#endif
					}
					else if (rip_table[i].metric >= in_dist) {
						rip_table[i].metric = in_dist;
						if (rip_table[i].nexthop.s_addr != from_addr) {
							rip_table[i].nexthop.s_addr = from_addr;
#ifndef RIP_MAIN
							modify_route(rip_table[i].addr.s_addr & rip_table[i].mask.s_addr, __builtin_ctz(~rip_table[i].mask.s_addr), lookfor24(from_addr)->name, rip_table[i].nexthop.s_addr);
#endif
						}
					}
					break;
				}
			}
			if (!appear) {
				TRtEntry newroute;
				newroute.addr = rip_in->entries[k].addr;
				newroute.mask = rip_in->entries[k].mask;
				newroute.nexthop.s_addr = from_addr;
				newroute.metric = min(htonl(rip_in->entries[k].metric) + 1, RIP_INFINITY);
				newroute.ifname = NULL;
				rip_table.push_back(newroute);
#ifndef RIP_MAIN
				insert_route(newroute.addr.s_addr & newroute.mask.s_addr, __builtin_ctz(~newroute.mask.s_addr), lookfor24(from_addr)->name, newroute.nexthop.s_addr);
#endif
			}
		}
	}
}

void* rip_recvpkt(void* args) {
	// 接收ip设置
	int sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) perror("Opening datagram socket error when receiving\n");
	else ;//printf("Opening datagram socket....OK.\n");
	// 防止绑定地址冲突, 设置地址重用
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
	} else ;//printf("Binding datagram socket...OK.\n");
	
	ip_mreq_source mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(RIP_GROUP);
	for (int i = 0; i < iface.size(); ++i) 
	if (iface[i].activate) {
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

	uint8_t buf[sizeof(TRipPkt)];
	TRipPkt* recvpkt = (TRipPkt*)buf;
	while(1) {
		// 接收rip报文   存储接收源ip地址
		// 判断command类型，request 或 response
		int32_t ret = recvfrom(sd, buf, sizeof(TRipPkt), 0, (sockaddr*)localSock, &sendsize);
		if (ret > 0) {
			printf("receive len: %d\tcmd: %d\tver: %d\tfrom %d.%d.%d.%d\n", ret, recvpkt->cmd, recvpkt->ver, TOIP(localSock->sin_addr.s_addr));
			rip_recv_handler(recvpkt, ret, localSock->sin_addr.s_addr);
		}
	}
}

void rip_timeout() {
	for (int i = 0; i < iface.size(); ++i)
		if (iface[i].activate)
			rip_timeout_handler(iface[i].addr);
	// puts("timeout!!!");
}

void* count_30s(void* args) {
	while (1) {
		rip_timeout();
		sleep(RIP_TIMEOUT * .5);
		get_local_info(false);
		sleep(RIP_TIMEOUT * .5);
	}
}

// 将本地接口表添加到rip路由表里
void get_local_info(bool init) {
	uint32_t localhost = inet_addr(init ? "1.0.0.0" : "127.0.0.1");
	ifaddrs *if_addr = NULL;	
	getifaddrs(&if_addr); // linux系统函数
	ifaddrs* head = if_addr;
	uint32_t groups = init ? AF_PACKET : AF_INET;
	if (!init)
		for (int i = 0; i < iface.size(); ++i)
			iface[i].activate = false;
	while (if_addr != NULL)
	{
		if (if_addr->ifa_addr->sa_family == groups)
		{
			uint32_t addr = ((sockaddr_in*)if_addr->ifa_addr)->sin_addr.s_addr;
			if (addr != localhost)
			{
				if (init) {
					// add interface
					Interface newface;
					newface.addr = addr;
					newface.name = new char[IF_NAMESIZE];
					newface.activate = false;
					strncpy(newface.name, if_addr->ifa_name, strlen(if_addr->ifa_name) + 1);
					iface.push_back(newface);
					// add to rip table
					TRtEntry newroute;
					newroute.mask.s_addr = 0xffffffff >> 8;
					newroute.addr.s_addr = newface.addr;// & newroute.mask.s_addr;
					newroute.nexthop.s_addr = 0;
					newroute.metric = 1;
					newroute.ifname = newface.name;
					rip_table.push_back(newroute);
				} else { // update interface
					printf("get %3d.%3d.%3d.%3d\tiface %s\n", TOIP(addr), if_addr->ifa_name);
					bool flag = false;
					for (int i = 0; i < iface.size() && !flag; ++i)
						if (strcmp(iface[i].name, if_addr->ifa_name) == 0) {
							flag = true;
							iface[i].addr = addr;
							iface[i].activate = true;
						}
				}
			}	
		}
		if_addr = if_addr->ifa_next;
	}
	if (!init) {
		printf("-----------------------------------------------------\n");
		printf("   |        Network       |   Nexthop Addr  | Metric \n");
		printf("-----------------------------------------------------\n");
		for (int i = 0; i < iface.size(); ++i) {
			if (iface[i].activate) {
				rip_table[i].metric = 1;
				rip_table[i].addr.s_addr = iface[i].addr;
			}
			else {
				rip_table[i].metric = 16;
				// rip_table[i].addr.s_addr = 0;
			}
			printf(" C | %3d.%3d.%3d.%3d / %2d | %3d.%3d.%3d.%3d | %4d\n", TOIP(rip_table[i].addr.s_addr), __builtin_ctz(~rip_table[i].mask.s_addr), TOIP(rip_table[i].nexthop.s_addr), rip_table[i].metric);
		}
		for (int i = iface.size(); i < rip_table.size(); ++i)
			printf(" R | %3d.%3d.%3d.%3d / %2d | %3d.%3d.%3d.%3d | %4d\n", TOIP(rip_table[i].addr.s_addr), __builtin_ctz(~rip_table[i].mask.s_addr), TOIP(rip_table[i].nexthop.s_addr), rip_table[i].metric);
		printf("-----------------------------------------------------\n");
	}
	freeifaddrs(head); // linux系统函数
}

void start_rip() {
	get_local_info(true);
	get_local_info(false);
	sleep(0.1);
	// 创建更新线程，30s更新一次,向组播地址更新Update包
	pthread_t p0, p1;
	int32_t pd0 = pthread_create(&p0, NULL, rip_recvpkt, NULL);
	int32_t pd1 = pthread_create(&p1, NULL, count_30s, NULL);
}

#ifdef RIP_MAIN
int main(int argc, char* argv[]) {
	start_rip();
	while(1) sleep(10000);
}
#endif