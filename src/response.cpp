#include "response.h"

Response::Response(HttpStatusCode code)
{
    status_line = "HTTP/1.1 " + http_code_map[code];
}

void Response::addHeader(HttpHeader header, std::string data)
{
    headers[header] = data;
}

void Response::addBody(std::string body)
{
    this->body = body;
}

std::string Response::toString()
{
    std::string result = status_line;
    result += "\r\n";

    for (auto &[key, value] : headers) {
        result += http_header_map[key] + ": " + value;
        result += "\r\n";
    }

    if (!body.empty()) {
        result += "\r\n";
        result += body;
    }

    return result;
}
