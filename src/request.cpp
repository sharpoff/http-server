#include "request.h"
#include "util.h"
#include <iomanip>

Request::Request()
{
}

std::ostream& operator<<(std::ostream &os, const Request &request)
{
    os << "method: " << std::quoted(request.method) << std::endl;
    os << "uri: " << std::quoted(request.uri) << std::endl;
    os << "version: " << std::quoted(request.version) << std::endl;
    for (auto &[key, value] : request.headers) {
        os << key << ": " << value << std::endl;
    }

    return os;
}

bool Request::parse(std::string data)
{
    auto lines = split(data, "\r\n");

    if (lines.empty()) return false;

    auto status = split(lines[0], " ");

    // TODO: validate method, uri and version
    if (status.size() != 3) return false;
    method = status[0];
    uri = status[1];
    version = status[2];

    // parse headers
    size_t i = 0;
    for (; i < lines.size(); i++) {
        auto line = lines[i];
        if (line.empty()) break;

        auto header = split(line, ": ");
        if (header.size() == 2) {
            headers[header[0]] = header[1];
        }
    }
    i++;

    // parse body
    // XXX: is this right?
    for (; i < lines.size(); i++) {
        body += lines[i];
    }

    return true;
}
