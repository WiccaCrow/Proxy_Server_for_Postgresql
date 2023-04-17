#include "Client.hpp"

#include "Server.hpp"

Client::Client(int fd_cli, int fd_db, int i_events) 
                : _i_events(i_events)
                , _fd_cli(fd_cli)
                , _fd_db(fd_db)
                , _id(-1)
                , _unlink(false) {
}

Client::~Client(void) {
    if (_fd_cli != -1) {
        if (_fd_cli == _fd_db) {
            _fd_db = -1;
        }
        close(_fd_cli);
        _fd_cli = -1;
    }
    if (_fd_db != -1) {
        close(_fd_db);
    }

    clearList(&_requests_cli);
    clearList(&_requests_db);

    clearList(&_responses_cli);
    clearList(&_responses_db);
}

void
Client::setId(int id) {
    _id = id;
}

void
Client::setUnlink(bool unlink) {
    _unlink = unlink;
}

int
Client::getId(void) {
    return _id;    
}

int     
Client::getFdCli(void) {
    return _fd_cli;
}

int     
Client::getFdDb(void) {
    return _fd_db;
}

bool
Client::getUnlink(void) const {
    return _unlink;
}

void Client::tryReceiveData(int fd) {

    listReq *req = (fd == _fd_cli ? &_requests_cli : &_requests_db);    
    addRequest(req);
    receive(req->back(), fd);
}

void 
Client::addRequest(listReq *reqlst) {
    Request *req = new Request(this);
    if (req == NULL) {
        ::std::cerr << "Cannot allocate memory for Request" << ::std::endl;
        return ;
    }
    reqlst->push_back(req);
}

int 
Client::addResponse(listRes *reslst) {
    Response *res = new Response();
    if (res == NULL) {
        ::std::cerr << "Cannot allocate memory for Response" << ::std::endl;
        return 1;
    }
    reslst->push_back(res);
    return 0;
}

void Client::receive(Request *req, int fd) {

    int bytes = read(fd);
        // ::std::cout << "Client::receive [" << fd << "] bytes " << bytes << ::std::endl; // test

    if (bytes < 0) {
        return ;

    } else if (bytes == 0) {
        ::std::cout << "Client::receive [" << fd << "] peer closed connection" << ::std::endl; // test
        setUnlink(true);
        return ;
    }

    while (!req->formed()) {
        std::string line;
        if (!getline(line, req->getExpBodySize() - req->getRealBodySize(), fd)) {
            return ;
        }

        req->parseLine(line);
    }
    // std::cout << "recieved request: is formed" << std::endl; // test
}

int
Client::getline(std::string &line, int64_t size, int fd) {
    std::size_t pos = 0;
    std::string *rem = (fd == _fd_cli ? &_rem_cli : &_rem_db);
    if (size < 0) {

        // если нет LF, читать дальше из fd
        // pos = rem->find(LF);
        pos = rem->length();
        if (pos == std::string::npos) {
            return 0;
        }
        // pos += 1;

    } else {
        if (rem->length() == 0 && size != 0) {
            return 0;
        }
        pos = size;
        if (pos > rem->length()) {
            pos = rem->length();
        }
    }
    line = rem->substr(0, pos);
    rem->erase(0, pos);
    return 1;
}

void
Client::makeResponse(int fd) {
    listReq  *reqlst = (fd == _fd_cli? &_requests_cli  : &_requests_db);
    listRes *reslst = (fd == _fd_cli? &_responses_db  : &_responses_cli);
    Request *req = reqlst->front();

    if (!addResponse(reslst)) {
        fillInResponse(reslst->front(), req);
    }
    removeRequest(fd);

    // ::std::cout << "makeResponse fd " << fd << "  |" << reslst->front()->getBody() << "| length:" << reslst->front()->getBody().length() << std::endl << std::endl << std::endl; // test
}

void
Client::fillInResponse(Response *res, Request *req) {
    res->setBody(req->getBody());
    res->formed(true);
}

int
Client::read(int fd) {

    char buf[BUFFER_SIZE + 1] = { 0 };

    int bytes = ::read(fd, buf, BUFFER_SIZE);
 
    if (bytes > 0) {
        buf[bytes] = '\0';
        // telnet: ctrl c, ctrl z
        if (!strcmp(buf, "\xff\xf4\xff\xfd\x06") || !strcmp(buf, "\xff\xed\xff\xfd\x06")) {
            return 0;
        }

        std::string *rem = (fd == _fd_cli ? &_rem_cli : &_rem_db);
        rem->append(buf, bytes);
        // std::cout << "rem length " << rem->length() << std::endl; // test
        return bytes;
    } 

    return bytes;
}

void
Client::tryReplyData(int fd) {
    listRes  *reslst = (fd == _fd_cli? &_responses_cli : &_responses_db);

    if (reslst->empty()) {
        return ;
    }

    Response *res = reslst->front();
    if (res->formed() && !res->sent()) {
        reply(fd, res);
    }

    if (res->formed() && res->sent())
        removeResponse(fd);
}

void
Client::removeRequest(int fd) {
    listReq *reqlst = (fd == _fd_cli ? &_requests_cli : &_requests_db);    

    if (reqlst->size() > 0) {
        Request *req = reqlst->front();
        reqlst->pop_front();
        delete req;
    }
}

void
Client::removeResponse(int fd) {
    listRes *reslst = (fd == _fd_cli ? &_responses_cli : &_responses_db);    

    if (reslst->size() > 0) {
        Response *res = reslst->front();
        reslst->pop_front();
        delete res;
    }
}

void
Client::reply(int fd, Response *res) {

    if (!res->sent()) {
        if (res->getBody().empty()) {
            res->sent(true);
            return ;
        }
    }

    int bytes = write(fd, res);
    
    if (bytes < 0)
        return ;
    
    if (bytes == 0) {
        ::std::cout << "Client:: [" << fd << "] peer closed connection" << ::std::endl;
        setUnlink(true);
        return ;
    }

    if (static_cast<std::size_t>(bytes) >= res->getBody().length() && !res->sent())
        res->sent(true);
}

int
Client::write(int fd, Response *res) {
    size_t pos = res->getBodyPos();

    long bytes = ::write(fd,  res->getBody().c_str() + pos, res->getBody().length() - pos);
    
    if (bytes > 0) {
        pos += bytes;
        res->setBodyPos(pos);
    
        if (pos >= res->getBody().length()) {
            // ::std::cout << "Client::write [" << fd << "]: " << pos << "/" << res->getBody().length() << " bytes" << ::std::endl; // test
            res->clear();
        }
    }

    return bytes;
}
