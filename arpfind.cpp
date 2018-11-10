#include "arpfind.h"

int32_t arpGet(arpmac* srcmac, nexthop* nexthopinfo)
{
	arpreq arp_req;
	sockaddr_in *sin = (sockaddr_in*)&(arp_req.arp_pa);
	memset(&arp_req, 0, sizeof(arp_req));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = nexthopinfo->nexthopaddr.s_addr;
	// eth1 is the name of interface of next hop
	strncpy(arp_req.arp_dev, nexthopinfo->ifname, IF_NAMESIZE - 1);
	int32_t arp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	int32_t ret = ioctl(arp_fd, SIOCGARP, &arp_req);// be careful with the return value!
	if (ret < 0) {
		fprintf(stderr, "get arp failed\n");
		return -1;
	}
	if (arp_req.arp_flags & ATF_COM) {
		srcmac->mac = new unsigned char[ETH_ALEN];
		// entry found
		// the mac address can be directed copied to eth_header->ether_dhost
		memcpy(srcmac->mac, (unsigned char *)arp_req.arp_ha.sa_data, sizeof(unsigned char) * ETH_ALEN);
		printf("Destination MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		srcmac->mac[0], srcmac->mac[1], srcmac->mac[2], srcmac->mac[3], srcmac->mac[4], srcmac->mac[5]);
	} else {
		fprintf(stderr, "mac entry not found\n");
		return -2;
	}
	// always remember to close it
	close(arp_fd);
	return 0;
}
