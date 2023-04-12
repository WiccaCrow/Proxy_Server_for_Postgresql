#pragma once

#include <sys/epoll.h>
// #include <exception>

#include <cstdlib>
#include <iostream>     // cout, cerr
#include <unistd.h>     // exit()
#include <vector>       // var
#include <sys/types.h>  // getadrinfo(), gai_strerror()
#include <sys/socket.h> // getadrinfo(), gai_strerror()
#include <netdb.h>      // getadrinfo(), gai_strerror()
#include <cstring>      // memset()
// #include <cstring>
// #include <list>
// #include <queue>

// Settings:
// 1. connection:
#define PORT  "8080"
#define IP    "127.0.0.1"
#define MAX_WAIT_CONNECTION 128
#define MAX_EVENTS 64
// 2. design:
#define COLOR_ERROR "\033[31m"
#define COLOR_END "\033[0m"

class Server {
    public:

    private:
    struct epoll_event event_server;
    ::std::vector<struct epoll_event> events;

    int     fd_listen;
    int     fd_epoll;
    int     listenSockfd(void);

    protected:
    void    clear(void);
    void    check_error(bool check, std::string error_msg, int exit_code);
    void    exit(std::string error_msg, int exit_code);

    public:
    // Settings settings;

    Server(void);
    ~Server(void);
    // Server(const Server &);
    // Server &operator=(const Server &);

    // ServersList &operator[](std::size_t port);
    // ServersMap  &getServerBlocks(void);

    void start(void);

};
