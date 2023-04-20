#pragma once

#include <sys/epoll.h>
// #include <exception>

#include <cstdlib>
#include <iostream>     // cout, cerr
#include <unistd.h>     // exit(), close()
#include <vector>       // var
#include <map>          // var
#include <sys/types.h>  // getadrinfo(), gai_strerror(), socket()
#include <sys/socket.h> // getadrinfo(), gai_strerror(), socket()
#include <netdb.h>      // getadrinfo(), gai_strerror()
#include <cstring>      // memset()
#include <algorithm>    // find_if()
#include <signal.h>     // signal()
#include <sys/socket.h> // inet_ntoa()
#include <netinet/in.h> // inet_ntoa()
#include <arpa/inet.h>  // inet_ntoa()

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
#define COLOR_ERROR "\033[35m"
#define COLOR_END "\033[0m"

class Server {
public:
    typedef std::vector<::Client *>       ClientsVec;
    typedef ClientsVec::iterator          iter_cv;

    typedef std::map<int, int>  FdIdMap;
    typedef FdIdMap::iterator   iter_fim;

private:

    bool        _working;
    struct epoll_event _event;
    ::std::vector<struct epoll_event> _events;
    ClientsVec  _clients;
    FdIdMap     _connector;
    int         _fd_listen;
    int         _fd_epoll;

    int     listenSockfd(void);

protected:
    void        clear(void);
    int         checkError(bool is_error, std::string error_msg, int exit_code);

    void        epollerr(int i);
    void        epollhup(int i);

    void        epollin(int fd);
    void        addNewClient(int i_events);
    int         fdClient(struct sockaddr_in &clientData);
    int         fdDatabase(void);
    int         setConnectionDb(struct addrinfo *lst);
    std::string clientIpPort(struct sockaddr_in &clientData);
    void        addClient(int fd_cli, int fd_db, ::Client *client);
    void        rmClient(Client *client);
    int         addFdsToEpoll(int fd, Client *client);

    void        epollout(int fd);

    Client *    getClient(int i);


public:
    Server(void);
    ~Server(void);

    void start(void);
    void unlink(Client *Cli);
    void finish(void);

};
