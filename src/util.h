#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

bool readFile(std::string filename, std::string &buffer);
std::vector<std::string> split(const std::string &s, const std::string &delim);
std::string getMimeType(std::filesystem::path file);
std::string getGMTTime();
