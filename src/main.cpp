#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

// net
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include "util.h"
#include "request.h"
#include "response.h"

#include "thread_pool.h"

#define PORT "8080"
#define MAX_CONNECTIONS 1
#define BUFSIZE 1024

void sigchldHandler(int s)
{
    // waitpid() sets errno, so save it
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *getInAddr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char *sockRecv(int fd)
{
    size_t bytes_read = 0;
    size_t cur_size = 0;
    char *buf = NULL;
    ssize_t status = 0;

    do {
        if (bytes_read >= cur_size) {
            cur_size += BUFSIZE;
            char *tmp = (char*)realloc(buf, cur_size);
            if (!tmp) break;

            buf = tmp;
        }

        status = recv(fd, buf + bytes_read, BUFSIZE, 0);
        if (status > 0) {
            bytes_read += status;
        }

        if (status < BUFSIZE) break;
    } while (status > 0);

    buf[cur_size - 1] = '\0';

    return buf;
}

bool sockSend(int fd, char *buf, size_t bufsize)
{
    if (!fd || !buf || bufsize <= 0) {
        return false;
    }

    int bytes_sent = 0;
    int size = 0;

    while ((bytes_sent = send(fd, buf + size, bufsize - size, 0)) > 0) {
        size += bytes_sent;
        if (size >= bufsize) break;
    }

    if (bytes_sent < 0) {
        perror("send");
        return false;
    }

    return true;
}

void handleConnection(int newfd)
{
    // request
    char *buf = sockRecv(newfd);

    std::string date = getGMTTime();

    Request request;
    std::string msg = "";

    std::string basepath = std::filesystem::current_path();

    if (request.parse(buf)) {
        // std::cout << "[REQUEST BEGIN]:\n" << request << "\n[REQUEST END]:\n" << "\n";

        // response 200
        std::filesystem::path path = "." + request.uri;
        if (std::filesystem::exists(path)) {

            if (std::filesystem::is_directory(path))
                path += "index.html";

            path = std::filesystem::absolute(std::filesystem::weakly_canonical(path));
        }

        std::string path_str = path.string();
        if (std::filesystem::exists(path) && path_str.substr(0, basepath.size()) == basepath) {
            Response response(HttpStatusCode::CODE_200);
            response.addHeader(HttpHeader::DATE, date);
            response.addHeader(HttpHeader::CONTENT_TYPE, getMimeType(path));

            // TODO: make it safe from directory traversal
            std::string body;
            if (readFile(path, body)) {
                response.addBody(body);
            }

            msg = response.toString();
        } else {
            Response response(HttpStatusCode::CODE_404);
            response.addHeader(HttpHeader::DATE, date);
            response.addHeader(HttpHeader::CONTENT_TYPE, getMimeType("index.html"));

            std::string body = "<html><body><h1>404 Not Found</h1></body></html>";
            response.addBody(body);

            msg = response.toString();
        }
    } else { // failed to parse request
        Response response(HttpStatusCode::CODE_500);
        response.addHeader(HttpHeader::DATE, date);
        response.addHeader(HttpHeader::CONTENT_TYPE, getMimeType("index.html"));

        std::string body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        response.addBody(body);

        msg = response.toString();
    }

    if (!msg.empty()) {
        if (sockSend(newfd, msg.data(), msg.size())) {
            // std::cout << "sent:\n" << msg << "\n";
        } else {
            std::cerr << "Failed to send a response.\n";
        }

    }

    free(buf);
    close(newfd);
}

int main()
{
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(NULL, PORT, &hints, &servinfo);
    if (ret != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(ret) << std::endl;
        return -1;
    }

    struct addrinfo *p;
    int sockfd; // server socket

    // find valid socket and bind to it
    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("server: socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            close(sockfd);
            return -1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        std::cerr << "Failed to bind socket\n";
        return -1;
    }

    if (listen(sockfd, MAX_CONNECTIONS) == -1) {
        perror("listen");
        return -1;
    }

    // configure zombie process termination
    struct sigaction sa;
    sa.sa_handler = sigchldHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    socklen_t addr_len;
    struct sockaddr_storage client_addr;
    int newfd;

    ThreadPool pool(std::thread::hardware_concurrency() - 2);

    while (1) {
        std::cout << "server: waiting for connections...\n";

        addr_len = sizeof(client_addr);
        newfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
        if (newfd == -1) {
            perror("accept");
            return -1;
        }

        char pnet_addr[INET6_ADDRSTRLEN];
        inet_ntop(client_addr.ss_family, getInAddr((struct sockaddr*)&client_addr), pnet_addr, sizeof(pnet_addr));
        std::cout << "server: connection from " << pnet_addr << std::endl;

        auto res = pool.enqueue([&]() { handleConnection(newfd); });
        res.get();
    }

    return 0;
}
