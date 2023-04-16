#include "Client.hpp"

#include "Server.hpp"

Client::Client(int fd_cli, int fd_db, int i_events) 
                : _i_events(i_events)
                , _fd_cli(fd_cli)
                , _fd_db(fd_db)
                , _id(-1) {
    

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
}

void
Client::set_id(int id) {
    _id = id;
}

void
Client::set_fd_cli(int fd) {
    _fd_cli = fd;
}

void
Client::set_fd_db(int fd) {
    _fd_db = fd;
}

int
Client::get_id(void) {
    return _id;    
}

int     
Client::get_fd_cli(void) {
    return _fd_cli;
}

int     
Client::get_fd_db(void) {
    return _fd_db;
}

int     
Client::get_i_events(void) {
    return _i_events;
}

void Client::tryReceiveRequest(int fd) {

    std::list<Request *> *req;
    if (fd == _fd_cli)
        req = &_requests_cli;
    else
        req = &_requests_db;
    
    addRequest(req);
    receive(req->back(), fd);
}

void 
Client::addRequest(std::list<Request *> *reqlst) {
    Request *req = new Request(this);
    if (req == NULL) {
        ::std::cerr << "Cannot allocate memory for Request" << ::std::endl;
        return ;
    }
    reqlst->push_back(req);
}

void Client::receive(Request *req, int fd) {

    int bytes = read(fd);

    if (bytes < 0) {
        return ;

    } else if (bytes == 0) {
        // ::std::cout << "Client::receive [" << get_fd_read() << "] peer closed connection" << ::std::endl;
        g_server.unlink(this);
        return ;
    }

    while (!req->formed()) {
        std::string line;
        if (!getline(line, req->getExpBodySize() - req->getRealBodySize(), fd)) {
            return ;
        }

        req->parseLine(line);
    }
    std::cout << "recieved request: is formed" << std::endl;

}

int
Client::getline(std::string &line, int64_t size, int fd) {
    std::size_t pos = 0;
    std::string *rem = (fd == _fd_cli ? &_rem_cli : &_rem_db);
    if (size < 0) {
        pos = rem->find(LF);
        // если нет LF, читать дальше из fd
        if (pos == std::string::npos) {
            return 0;
        }
        pos += 1;

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
        return bytes;
    } 

    return bytes;
}
