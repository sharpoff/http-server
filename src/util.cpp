#include "util.h"
#include <fstream>

bool readFile(std::string filename, std::string &buffer)
{
    if (!std::filesystem::exists(filename)) return false;

    std::ifstream file(filename);
    if (file.bad()) return false;

    auto size = std::filesystem::file_size(filename);
    buffer.resize(size, '\0');
    file.read(&buffer[0], size);

    return true;
}

std::vector<std::string> split(const std::string &s, const std::string &delim)
{
    std::vector<std::string> tokens;
    size_t end = 0, start = 0, delim_len = delim.length();

    std::string token;
    while ((end = s.find(delim, start)) != std::string::npos) {
        token = s.substr(start, end - start);

        tokens.push_back(token);
        start = end + delim_len;
    }
    tokens.push_back(s.substr(start, end));

    return tokens;
}

std::string getMimeType(std::filesystem::path file)
{
    std::string extension = file.extension();

    if (extension == ".html" || extension == ".htm") return "text/html";
    else if (extension == ".txt") return "text/plain";
    else if (extension == ".ico") return "image/vnd.microsoft.icon";
    else if (extension == ".jpeg" || extension == ".jpg") return "image/jpeg";
    else if (extension == ".csv") return "text/csv";
    else if (extension == ".pdf") return "application/pdf";
    else if (extension == ".epub") return "application/epub+zip";
    else if (extension == ".docx") return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    else if (extension == ".doc") return "application/msword";
    else if (extension == ".json") return "application/json";
    else if (extension == ".gif") return "image/gif";
    else if (extension == ".png") return "image/png";
    else if (extension == ".mp3") return "audio/mpeg";
    else if (extension == ".mp4") return "video/mp4";
    else if (extension == ".mpeg") return "video/mpeg";

    return "application/octet-stream";
}

std::string getGMTTime()
{
    const std::time_t now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now), "%a, %d %h %Y %H:%M:%S GMT");

    return ss.str();
}
