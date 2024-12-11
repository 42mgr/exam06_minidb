#pragma once
#include <string>
#include <istream>

class Processing {
	std::string response;
	void handle_get(std::string key);
	void handle_delete(std::string key);
	void handle_post(std::string key, std::string value);
public:
	Processing(std::istream& input);
	std::string getResponse();
};
