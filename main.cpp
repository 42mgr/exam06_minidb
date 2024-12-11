
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include "Server.hpp"
#include "Socket.hpp"
#include "global.h"

// Global variables for signal handling
std::unordered_map<std::string, std::string> database;
std::string save_path;
bool running = true;
int server_socket = -1;

// Signal handler for SIGINT
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::ofstream file(save_path.c_str());
        if (file.is_open()) {
            std::unordered_map<std::string, std::string>::iterator it;
            for (it = database.begin(); it != database.end(); ++it) {
                file << it->first << " " << it->second << "\n";
            }
            file.close();
        }
        running = false;
        if (server_socket >= 0) {
            close(server_socket);
        }
    }
}

// Load database from file
void load_database(const std::string& path) {
    std::ifstream file(path.c_str());
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
    std::unordered_map<std::string, std::string>::iterator it = database.find(key);
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

std::string process_commands(std::istream& input) {
    std::string result;
    std::string line;
    
    while (std::getline(input, line)) {
        std::istringstream iss(line);
        std::string cmd, key, value;
        
        iss >> cmd;
        
        if (cmd == "POST") {
            if (iss >> key >> value) {
                result += handle_post(key, value) + "\n";
            } else {
                result += "2\n";
            }
        } else if (cmd == "GET") {
            if (iss >> key) {
                result += handle_get(key) + "\n";
            } else {
                result += "2\n";
            }
        } else if (cmd == "DELETE") {
            if (iss >> key) {
                result += handle_delete(key) + "\n";
            } else {
                result += "2\n";
            }
        } else {
            result += "2\n";
        }
    }
    
    return result;
}
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <save_path>\n";
        return 1;
    }

    std::istringstream iss(argv[1]);
    int port;
    iss >> port;
    save_path = argv[2];

    // Load existing database
    load_database(save_path);

    // Set up signal handler
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        std::cerr << "Failed to set up signal handler\n";
        return 1;
    }

    Server server(port);
    server_socket = server.run();
}
