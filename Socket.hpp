#pragma once

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

class Socket {
private:
	int sockfd;
	struct sockaddr_in serveraddr;

public:
	Socket(int port);
	~Socket();
	void bindAndListen();
	int accept_connection(struct sockaddr_in& clientAddr);
	std::string pullMessage(int clientSockFd);
	int getServerFd();
};
