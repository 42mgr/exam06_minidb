#include "Server.hpp"
#include "Processing.hpp"

Server::Server(int port) : listeningSocket(port) {}

int Server::run() {
    listeningSocket.bindAndListen();
    std::cout << "Server is listening..." << std::endl;

    fd_set master_fds;
    fd_set read_fds;
    int max_fd = listeningSocket.getServerFd();

    FD_ZERO(&master_fds);
    FD_SET(listeningSocket.getServerFd(), &master_fds);

    while (true) {
        read_fds = master_fds;
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            throw std::runtime_error("Select failed");
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (!FD_ISSET(fd, &read_fds)) {
                continue;
            }

            if (fd == listeningSocket.getServerFd()) { // Handle new connection
                struct sockaddr_in client_addr;
                int new_fd = listeningSocket.accept_connection(client_addr);
                
                if (new_fd != -1) {
                    FD_SET(new_fd, &master_fds);
                    if (new_fd > max_fd) {
                        max_fd = new_fd;
                    }
                    std::cout << "New client connected" << std::endl;
                }
            } else {
                // Handle client data
                std::string message = listeningSocket.pullMessage(fd);
                if (message.empty()) {
                    // Connection closed
                    close(fd);
                    FD_CLR(fd, &master_fds);
                } else {
                    // Process message
                    std::cout << "Received message from client: " << message << std::endl;
                    std::istringstream iss(message);
                    Processing processing(iss);
                    message = processing.getResponse();
                    send(fd, &message, message.length(), 0);
                }
            }
        }
    }
    return 0;
}
