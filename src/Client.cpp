#include "Client.hpp"

#include "Server.hpp"

Client::Client(int fd_cli, int fd_db, int i_events) : 
                _i_events(i_events),
                _fd_cli(fd_cli),
                _fd_db(fd_db),
                _id(-1) {
    

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

    if (_requests.size() == _responses.size()) {
        addRequest();
    }

    if (_requests.size() > _responses.size()) {
        receive(_requests.back());
    }

    (void)fd;
    // if (_requests.back()->formed()) {
    //     addResponse();
    // }
}

void Client::addRequest(void) {

    Request *req = new Request();

    if (req == NULL) {
        ::std::cerr << "Cannot allocate memory for Request" << ::std::endl;
        return ;
    }
    _requests.push_back(req);
}

void Client::receive(Request *req) {

    int bytes = read();

    if (bytes < 0) {
        return ;

    } else if (bytes == 0) {
        // ::std::cout << "Client::receive [" << get_fd_read() << "] peer closed connection" << ::std::endl;
        g_server.unlink(this);
        return ;
    }

    while (!req->formed()) {
        std::string line;
std::cout << "test try recieve request" << std::endl;
        // if (!getClientIO()->getline(line, req->getExpBodySize() - req->getRealBodySize())) {
        //     return ;
        // }

        // req->parseLine(line);
    }
}

int 
Client::read(void) {

    char buf[BUFFER_SIZE + 1] = { 0 };

    int bytes = ::read(_fd_cli, buf, BUFFER_SIZE);
 
    if (bytes > 0) {
        buf[bytes] = '\0';
        // telnet: ctrl c, ctrl z
        if (!strcmp(buf, "\xff\xf4\xff\xfd\x06") || !strcmp(buf, "\xff\xed\xff\xfd\x06")) {
            return 0;
        }
        
        _rem.append(buf, bytes);
        return bytes;
    } 

    return bytes;
}
