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
        fprintf(stderr, "while\n");
		int conn_fd = accept(sock_fd, (sockaddr*)NULL, NULL);
        fprintf(stderr, "accept\n");
		recv(conn_fd, selfrt, sizeof(selfroute), 0);
        fprintf(stderr, "recv %d\n",selfrt->cmdnum);
		// do something with selfrt
		if (selfrt->cmdnum == 24 || selfrt->cmdnum == 25) {
            close(conn_fd);
			return 1;
        }
	    close(conn_fd);	
	}
}
