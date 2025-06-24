#pragma once

#include <string>
#include <unordered_map>

enum class HttpStatusCode
{
    UNDEFINED = 0,
    CODE_200,
    CODE_201,
    CODE_301,
    CODE_302,
    CODE_400,
    CODE_403,
    CODE_404,
    CODE_500,
    CODE_503,
};

static std::unordered_map<HttpStatusCode, std::string> http_code_map = {
    {HttpStatusCode::CODE_200, "200 OK"},
    {HttpStatusCode::CODE_201, "201 Created"},
    {HttpStatusCode::CODE_301, "301 Moved Permanently"},
    {HttpStatusCode::CODE_302, "302 Found"},
    {HttpStatusCode::CODE_400, "400 Bad Request"},
    {HttpStatusCode::CODE_403, "403 Forbidden"},
    {HttpStatusCode::CODE_404, "404 Not Found"},
    {HttpStatusCode::CODE_500, "500 Internal Server Error"},
    {HttpStatusCode::CODE_503, "503 Service Unavailable"},
};

enum class HttpHeader
{
    UNDEFINED = 0,
    CONTENT_TYPE,
    CONTENT_LENGTH,
    SERVER,
    DATE,
    LAST_MODIFIED,
    CONNECTION,
    LOCATION,
};

static std::unordered_map<HttpHeader, std::string> http_header_map = {
    {HttpHeader::CONTENT_TYPE, "Content-Type"},
    {HttpHeader::CONTENT_LENGTH, "Content-Length"},
    {HttpHeader::SERVER, "Server"},
    {HttpHeader::DATE, "Date"},
    {HttpHeader::LAST_MODIFIED, "Last-Modified"},
    {HttpHeader::CONNECTION, "Connection"},
    {HttpHeader::LOCATION, "Location"},
};

class Response
{
public:
    Response(HttpStatusCode code);

    void addHeader(HttpHeader header, std::string data);
    void addBody(std::string body);
    std::string toString();

private:
    std::string status_line;
    std::unordered_map<HttpHeader, std::string> headers;
    std::string body;
};
