#include "rip.h"

TRtEntry *g_pstRouteEntry = NULL;
TRipPkt *requestpkt = NULL;
TRipPkt *recvpkt = NULL;

char *pcLocalAddr[IF_NAMESIZE]={};//存储本地接口ip地址
char *pcLocalName[IF_NAMESIZE]={};//存储本地接口的接口名

void requestpkt_Encapsulate()
{
	// 封装请求包  command = 1, version = 2, family = 0, metric = 16
	TRtEntry *p = g_pstRouteEntry;
	int32_t cnt = 0;
	while (p != NULL) {
		int32_t index = cnt % RIP_MAX_ENTRY;
		if (index == 0) {
			requestpkt = new TRipPkt();
			requestpkt->ucCommand = RIP_REQUEST;
			requestpkt->ucVersion = RIP_VERSION;
			requestpkt->usZero = 0;
		}
		// 封装到RIP报文表项的结构体
		TRipEntry *ripentry = new TRipEntry();
		ripentry->stAddr = p->stIpPrefix;
		ripentry->stNexthop = p->stNexthop;
		ripentry->stPrefixLen.s_addr = (0xffffffff >> (32 - p->uiPrefixLen)) << (32 - p->uiPrefixLen);
		ripentry->uiMetric = RIP_INFINITY;
		ripentry->usFamily = 0;

		//封装好的rip报文
		requestpkt->RipEntries[index] = *ripentry;
		++cnt;
		p = p->pstNext;
	}
}


/*****************************************************
* Description:  接收rip报文
*******************************************************/
void rippacket_Receive()
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
		return;
	}
	// 设置端口重用
	int iReUsePort = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, (const char*)&iReUsePort, sizeof(iReUsePort)) < 0) {
		perror("setsockopt reuseport error\n");
		return;
	}
	// 把本地地址加入到组播
	sockaddr_in* localSock = new sockaddr_in();
	localSock->sin_family = AF_INET;
	localSock->sin_port = htons(RIP_PORT);
	localSock->sin_addr.s_addr = INADDR_ANY;
	if (bind(sd, (sockaddr*)localSock, sizeof(sockaddr_in))) {
		perror("Binding datagram socket error\n");
		close(sd);
		exit(1);
	} else printf("Binding datagram socket...OK.\n");

	listen(sd, 5);
	recvpkt = new TRipPkt();
	int cnt = 0;
	while(1) {
		// 接收rip报文   存储接收源ip地址
		// 判断command类型，request 或 response
		int conn_fd = accept(sd, (sockaddr*)NULL, NULL);
		int ret = recv(conn_fd, recvpkt, sizeof(TRipPkt), 0);
		close(conn_fd);
		if (ret > 0) {
			printf("cnt: %d\tucCammand: %s\tucVersion: %s\n", ++cnt, recvpkt->ucCommand, recvpkt->ucVersion);
		}
		// 接收到的信息存储到全局变量里，方便request_Handle和response_Handle处理
	}
}


/*****************************************************
*Func Name:    rippacket_Send  
*Description:  向接收源发送响应报文
*Input:        
*	  1.stSourceIp    ：接收源的ip地址，用于发送目的ip设置
*Output: 
*
*Ret  ：
*
*******************************************************/
void rippacket_Send(struct in_addr stSourceIp)
{
	//本地ip设置

	//发送目的ip设置
	
	/*防止绑定地址冲突，仅供参考
	//设置地址重用
	int  iReUseddr = 1;
	if (setsockopt(iSendfd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&iReUseddr,sizeof(iReUseddr))<0)
	{
		perror("setsockopt\n");
		return ;
	}
	//设置端口重用
	int  iReUsePort = 1;
	if (setsockopt(iSendfd,SOL_SOCKET ,SO_REUSEPORT,(const char*)&iReUsePort,sizeof(iReUsePort))<0)
	{
		perror("setsockopt\n");
		return ;
	}
	//把本地地址加入到组播中 	
	*/

	//创建并绑定socket

	//发送
	return;	
}

/*****************************************************
*Func Name:    rippacket_Multicast  
*Description:  组播请求报文
*Input:        
*	  1.pcLocalAddr   ：本地ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void rippacket_Multicast(char *pcLocalAddr)
{
	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		perror("Opening datagram socket error when multicasting\n");
		exit(1);
	} else {
		printf("Opening datagram socket....OK.\n");
	}

	// 防止绑定地址冲突，仅供参考
	// 设置地址重用
	int  iReUseddr = 1;
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

	// 绑定socket
	struct sockaddr_in router;
	router.sin_family = AF_INET;
	router.sin_port = htons(RIP_PORT);
	router.sin_addr.s_addr = inet_addr(RIP_GROUP);
	if(setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &pcLocalAddr, sizeof(pcLocalAddr)) < 0) {
		perror("Setting local interface error");
	} else printf("Setting the local interface...OK\n");

	int result;
	char buf[RIP_MAX_PACKET];
	memcpy(buf, &requestpkt, sizeof(TRipPkt));
	if ((result = sendto(fd, buf, sizeof(buf), 0, (sockaddr*)&router, sizeof(router))) == -1) {
		printf("send error!\n");
	} else {
		printf("send succeed! packet len: %d\n", result);
	}
	close(fd);
	// 发送
}

/*****************************************************
*Func Name:    request_Handle  
*Description:  响应request报文
*Input:        
*	  1.stSourceIp   ：接收源的ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void request_Handle(struct in_addr stSourceIp)
{
	//处理request报文
	//遵循水平分裂算法
	//回送response报文，command置为RIP_RESPONSE
	return;
}

/*****************************************************
*Func Name:    response_Handle  
*Description:  响应response报文
*Input:        
*	  1.stSourceIp   ：接收源的ip地址
*Output: 
*
*Ret  ：
*
*******************************************************/
void response_Handle(struct in_addr stSourceIp)
{
	//处理response报文
	return;
}

/*****************************************************
*Func Name:    route_SendForward  
*Description:  响应response报文
*Input:        
*	  1.uiCmd        ：插入命令
*	  2.pstRtEntry   ：路由信息
*Output: 
*
*Ret  ：
*
*******************************************************/
void route_SendForward(unsigned int uiCmd,TRtEntry *pstRtEntry)
{
	//建立tcp短连接，发送插入、删除路由表项信息到转发引擎
	return;
}

void rippacket_Update()
{
	//遍历rip路由表，封装更新报文
	//注意水平分裂算法
}

void ripdaemon_Start()
{
	//创建更新线程，30s更新一次,向组播地址更新Update包

	//封装请求报文，并组播
	requestpkt_Encapsulate();
	int i = 0;
	for (i = 0; i < 10; i++) {
		if (pcLocalAddr[i] != NULL) {
			rippacket_Multicast(pcLocalAddr[i]);
		} else break;
	}
	
	//接收rip报文
	rippacket_Receive();
}

void routentry_Insert()
{
	//将本地接口表添加到rip路由表里
	int i = 0;
	TRtEntry *p = g_pstRouteEntry;
	while (1) {
		if (i < 10 && pcLocalAddr[i] != NULL) {
			TRtEntry* newrip = new TRtEntry();
			newrip->pcIfname = pcLocalName[i];
			newrip->uiPrefixLen = 24;
			newrip->uiMetric = 0;
			newrip->stIpPrefix.s_addr = inet_addr(pcLocalAddr[i]);
			newrip->pstNext = NULL;

			if (g_pstRouteEntry == NULL) {
				g_pstRouteEntry = newrip;
				p = g_pstRouteEntry;
			} else {
				g_pstRouteEntry->pstNext = newrip;
				g_pstRouteEntry = g_pstRouteEntry->pstNext;
			}
			i++;
		} else break;
	}
	g_pstRouteEntry = p;
}

void localinterf_GetInfo()
{
	struct ifaddrs *pstIpAddrStruct = NULL;
	struct ifaddrs *pstIpAddrStCur  = NULL;
	void *pAddrPtr=NULL;
	const char *pcLo = "127.0.0.1";
	
	getifaddrs(&pstIpAddrStruct); // linux系统函数
	pstIpAddrStCur = pstIpAddrStruct;
	
	int i = 0;
	while(pstIpAddrStruct != NULL)
	{
		if(pstIpAddrStruct->ifa_addr->sa_family==AF_INET)
		{
			pAddrPtr = &((struct sockaddr_in *)pstIpAddrStruct->ifa_addr)->sin_addr;
			char cAddrBuf[INET_ADDRSTRLEN];
			memset(&cAddrBuf,0,sizeof(INET_ADDRSTRLEN));
			inet_ntop(AF_INET, pAddrPtr, cAddrBuf, INET_ADDRSTRLEN);
			if(strcmp((const char*)&cAddrBuf,pcLo) != 0)
			{
				pcLocalAddr[i] = (char *)malloc(sizeof(INET_ADDRSTRLEN));
				pcLocalName[i] = (char *)malloc(sizeof(IF_NAMESIZE));
				strcpy(pcLocalAddr[i],(const char*)&cAddrBuf);
				strcpy(pcLocalName[i],(const char*)pstIpAddrStruct->ifa_name);
				i++;
			}	
		}
		pstIpAddrStruct = pstIpAddrStruct->ifa_next;
	}
	freeifaddrs(pstIpAddrStCur); // linux系统函数
}

int main(int argc, char* argv[])
{
	g_pstRouteEntry = new TRtEntry();
	localinterf_GetInfo();
	routentry_Insert();
	ripdaemon_Start();
	return 0;
}

