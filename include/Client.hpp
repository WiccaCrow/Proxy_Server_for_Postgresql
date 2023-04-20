#pragma once

#include <unistd.h>     // fcntl(), close()
#include <fcntl.h>      // fcntl()
#include <cstddef>      // size_t
#include <list>         // var
#include <iterator>     // для итераторов
#include <iostream>     // cerr
#include <cstring>      // strcmp()

#include "Request.hpp"
#include "Response.hpp"
#include "globals.hpp"

static const std::size_t BUFFER_SIZE = 65536;

class Client {
    
public:
    typedef std::list<Request *>  listReq;
    typedef std::list<Request *>::iterator  listReqIter;
    typedef std::list<Response *> listRes;
    typedef std::list<Response *>::iterator listResIter;

private:
    std::string _clientHostPort;
    int         _i_events;
    int         _fd_cli;
    int         _fd_db;
    int         _id;
    bool        _unlink;

    std::string _rem_cli;
    std::string _rem_db;

    listReq     _requests_cli;
    listReq     _requests_db;

    listRes     _responses_cli;
    listRes     _responses_db;

    std::string _cli_ip_port;

    void    addRequest(listReq *reqlst);
    void    receive(Request *req, int fd);
    int     read(int fd);
    int     getline(std::string &line, int64_t size, int fd);

    int     addResponse(listRes *reslst);
    void    fillInResponse(Response *res, Request *req);
    int     write(int fd, Response *res);

    void    logs(int fd, std::string str, int isClosed);
    bool    checkDeepLogs(int fd);
    void    logsHostPortFd(int fd, int isClosed);


    template<class TList>
    void    clearList(TList *clean) {
        for (auto it = clean->begin(); it != clean->end(); ++it)
            if (*it != NULL) 
                delete *it;
    }

    
public:
    explicit Client(std::string clientHostPort, int fd_cli, int fd_db, int i_events);
            ~Client(void);

    void    setId(int id);
    void    setUnlink(bool unlink);

    int     getId(void);
    int     getFdCli(void);
    int     getFdDb(void);
    bool    getUnlink(void)    const;

    void    tryReceiveData(int fd);
    void    makeResponse(int fd);
    void    removeRequest(int fd);

    void    tryReplyData(int fd);
    void    reply(int fd, Response *res);
    void    removeResponse(int fd);

};
