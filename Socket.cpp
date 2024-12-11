#include "Socket.hpp"

Socket::Socket(int port) : sockfd(socket(AF_INET, SOCK_STREAM, 0)) {
  if (sockfd == -1) {
    throw std::runtime_error("Failed to create socket");
  }
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(port);
}

Socket::~Socket() {
  if (sockfd != -1)
    close(sockfd);
}

int Socket::getServerFd() {
  return sockfd;
}

void Socket::bindAndListen() {
  try {
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
      throw std::runtime_error("Bind failed");
    }
    if (listen(sockfd, 10) < 0) {
      throw std::runtime_error("Listen failed");
    }
  } catch (const std::exception &e) {
    std::cerr << "bindAndListen() error: " << e.what() << std::endl;
    throw;
  }
}

int Socket::accept_connection(struct sockaddr_in &clientAddr) {
  socklen_t clientLen = sizeof(clientAddr);
  int clientSockFd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientLen);
  if (clientSockFd < 0) {
    throw std::runtime_error("Accept failed");
  }
  return clientSockFd;
}

std::string Socket::pullMessage(int clientSockFd) {
  char buffer[1024];
  ssize_t bytesRead = recv(clientSockFd, buffer, sizeof(buffer) - 1, 0);
  if (bytesRead < 0) {
    throw std::runtime_error("Receive failed");
  }
  buffer[bytesRead] = '\0';
  return std::string(buffer);
}
