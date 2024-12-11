#pragma once 
#include "Socket.hpp"
#include <arpa/inet.h>
#include <stdexcept>
#include <istream>
#include <sstream>

class Server {
private:
public:
	int maxfd;
	Socket listeningSocket;
	Server(int port);
	int run();
};
