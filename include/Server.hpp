#pragma once

#include <sys/epoll.h>
// #include <exception>

#include <cstdlib>
#include <iostream>     // cout, cerr
#include <unistd.h>     // exit(), close()
#include <vector>       // var
#include <sys/types.h>  // getadrinfo(), gai_strerror()
#include <sys/socket.h> // getadrinfo(), gai_strerror()
#include <netdb.h>      // getadrinfo(), gai_strerror()
#include <cstring>      // memset()
#include <algorithm>    // find_if()
// #include <cstring>
// #include <list>
// #include <queue>

#include "Client.hpp"
#include "utils.hpp"

// Settings:
// 1. connection:
#define PORT  "8080"
#define IP    "127.0.0.1"
#define MAX_WAIT_CONNECTION 128
#define MAX_EVENTS 64
// 2. design:
#define COLOR_ERROR_FATAL "\033[31m"
#define COLOR_ERROR_EPOL "\033[35m"
#define COLOR_END "\033[0m"

class Server {
    public:
    typedef std::vector<::Client *>       ClientsVec;
    typedef ClientsVec::iterator          iter_cv;

    private:
    struct epoll_event event;
    ::std::vector<struct epoll_event> events;

    ClientsVec   _clients;


    int     fd_listen;
    int     fd_epoll;
    int     listenSockfd(void);

    protected:
    void    clear(void);
    void    check_error(bool check, std::string error_msg, int exit_code);
    void    exit(::std::string error_msg, int exit_code);

    void    epollerr(int i);
    void    epollhup(int i);
    void    epoll_err_msg(::std::string msg);

    void    epollin(int i);
    void    add_new_client(void);
    void    addClient(::Client *client);

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
