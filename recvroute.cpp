#include "recvroute.h"

int32_t static_route_get(selfroute *selfrt)
{
	sockaddr_in server_addr;
	int32_t sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&server_addr, sizeof(sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(800);
	bind(sock_fd, (sockaddr*)(&server_addr), sizeof(sockaddr));
	listen(sock_fd, 5);
	while (1) {
		int conn_fd = accept(sock_fd, (sockaddr*)NULL, NULL);
		recv(conn_fd, selfrt, sizeof(selfroute), 0);
		close(conn_fd);
		// do something with selfrt
		if (selfrt->cmdnum == 24 || selfrt->cmdnum == 25)
			return 1;
	}
}