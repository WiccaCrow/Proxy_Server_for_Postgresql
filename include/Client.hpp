#pragma once

#include <unistd.h>     // fcntl(), close()
#include <fcntl.h>      // fcntl()
#include <cstddef>      // size_t
#include <list>         // var
#include <iostream>     // cerr
#include <cstring>      // strcmp()

#include "Request.hpp"
#include "Response.hpp"
#include "globals.hpp"

static const std::size_t BUFFER_SIZE = 65536;

class Client {
    public:
    explicit Client(int fd_cli, int fd_db, int i_events);
            ~Client(void);

    int     nonblock(void);
    void    set_id(int id);
    void    set_fd_cli(int fd);
    void    set_fd_db(int fd);

    int     get_id(void);
    int     get_fd_cli(void);
    int     get_fd_db(void);
    int     get_i_events(void);

    void    tryReceiveRequest(int fd);


    private:
    int     _i_events;
    int     _fd_cli;
    int     _fd_db;
    int     _id;

    std::string _rem;

    std::list<Request *>  _requests;
    std::list<Response *> _responses;

    void    addRequest(void);
    void    receive(Request *req);
    int     read(void);

};