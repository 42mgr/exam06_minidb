#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>

// Global variables for signal handling
std::unordered_map<std::string, std::string> database;
std::string save_path;
bool running = true;

// Signal handler for SIGINT
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::ofstream file(save_path);
        if (file.is_open()) {
            for (const auto& pair : database) {
                file << pair.first << " " << pair.second << "\n";
            }
            file.close();
        }
        running = false;
    }
}

// Load database from file
void load_database(const std::string& path) {
    std::ifstream file(path);
    if (file.is_open()) {
        std::string key, value;
        while (file >> key >> value) {
            database[key] = value;
        }
        file.close();
    }
}

// Handle POST command
std::string handle_post(const std::string& key, const std::string& value) {
    database[key] = value;
    return "0\n";
}

// Handle GET command
std::string handle_get(const std::string& key) {
    auto it = database.find(key);
    if (it != database.end()) {
        return "0 " + it->second + "\n";
    }
    return "1\n";
}

// Handle DELETE command
std::string handle_delete(const std::string& key) {
    if (database.erase(key) > 0) {
        return "0\n";
    }
    return "1\n";
}

// Process client command
std::string process_command(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd, key, value;
    
    iss >> cmd;
    
    if (cmd == "POST") {
        if (iss >> key >> value) {
            return handle_post(key, value);
        }
    } else if (cmd == "GET") {
        if (iss >> key) {
            return handle_get(key);
        }
    } else if (cmd == "DELETE") {
        if (iss >> key) {
            return handle_delete(key);
        }
    }
    
    return "2\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <save_path>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    save_path = argv[2];

    // Load existing database
    load_database(save_path);

    // Set up signal handler
    signal(SIGINT, signal_handler);

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Setsockopt failed\n";
        return 1;
    }

    // Configure server address
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << "\n";
        return 1;
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    // Set of client sockets
    fd_set master_fds;
    fd_set read_fds;
    int max_fd = server_fd;

    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);

    while (running) {
        read_fds = master_fds;
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            if (running) {
                std::cerr << "Select failed\n";
            }
            break;
        }

        // Check all possible file descriptors
        for (int fd = 0; fd <= max_fd; fd++) {
            if (!FD_ISSET(fd, &read_fds)) {
                continue;
            }

            if (fd == server_fd) {
                // Handle new connection
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                
                if (new_fd == -1) {
                    if (running) {
                        std::cerr << "Accept failed\n";
                    }
                } else {
                    FD_SET(new_fd, &master_fds);
                    if (new_fd > max_fd) {
                        max_fd = new_fd;
                    }
                }
            } else {
                // Handle data from client
                char buffer[1001] = {0};
                ssize_t bytes_read = read(fd, buffer, 1000);
                
                if (bytes_read <= 0) {
                    // Connection closed or error
                    close(fd);
                    FD_CLR(fd, &master_fds);
                } else {
                    // Process command and send response
                    buffer[bytes_read] = '\0';
                    std::string command(buffer);
                    if (!command.empty() && command[command.length()-1] == '\n') {
                        command.pop_back();
                    }
                    
                    std::string response = process_command(command);
                    send(fd, response.c_str(), response.length(), 0);
                }
            }
        }
    }

    // Cleanup
    for (int fd = 0; fd <= max_fd; fd++) {
        if (FD_ISSET(fd, &master_fds)) {
            close(fd);
        }
    }
    return 0;
}
