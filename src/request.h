#pragma once

#include <unordered_map>
#include <string>
#include <iostream>

class Request
{
public:
    Request();

    bool parse(std::string data);
    friend std::ostream& operator<<(std::ostream &os, const Request &request);

    std::string method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body; // could be empty
};
