#include "Processing.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include "global.h"

Processing::Processing(std::istream& input) : response("2"){
    std::string line;

    while(std::getline(input, line)) {
    std::istringstream iss(line);
    std::string cmd, key, value;

    iss >> cmd;

    if (cmd == "POST") {
        std::cout << "Command is POST" << std::endl;
        if (iss >> key >> value)
            Processing::handle_post(key, value);
    }
    else if (cmd == "GET") {
        std::cout << "Command is GET" << std::endl;
        if (iss >> key)
            Processing::handle_get(key);
    }
    else if (cmd == "DELETE") {
        std::cout << "Command is DELETE" << std::endl;
        if (iss >> key)
            Processing::handle_delete(key);
    }}
}


std::string Processing::getResponse() {
    return response + "\n";
}

void Processing::handle_post(std::string key, std::string value) {
    database[key] = value;
    response = "0";
}

void Processing::handle_delete(std::string key) {
    if (database.erase(key) > 0)
        response = "0";
    else    
        response = "1";
}

void Processing::handle_get(std::string key) {
    auto it = database.find(key);
    if (it != database.end())
        response = "0 " + it->second;
    else
        response = "1";
}
