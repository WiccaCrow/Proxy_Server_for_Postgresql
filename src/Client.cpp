#include "Client.hpp"
#include "Server.hpp"
#include "ParserCli.hpp"

// Constructors and destructor:


Client::Client(std::string clientHostPort, int fd_cli, int fd_db, int i_events) 
                : _clientHostPort(clientHostPort)
                , _i_events(i_events)
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



// private:



/* void    addRequest(listReq *reqlst);
* Adds a new empty Request object to the end of the list listReq 
* (_requests_cli or _requests_db).
*/

void 
Client::addRequest(listReq *reqlst) {
    Request *req = new Request(this);
    if (req == NULL) {
        ::std::cerr << "Cannot allocate memory for Request" << ::std::endl;
        return ;
    }
    reqlst->push_back(req);
}

/* void receive(Request *req, int fd); 
* Receive data from clients or data base.
*/

void
Client::receive(Request *req, int fd) {

    int bytes = read(fd);
        // ::std::cout << "Client::receive [" << fd << "] bytes " << bytes << ::std::endl; // test

    if (bytes < 0) {
        return ;

    } else if (bytes == 0) {
        logs(fd, "peer closed connection", true);
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

int
Client::getline(std::string &line, int64_t size, int fd) {
    std::size_t pos = 0;
    std::string *rem = (fd == _fd_cli ? &_rem_cli : &_rem_db);
    if (size < 0) {
        pos = rem->length();
        if (pos == std::string::npos) {
            return 0;
        }
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
Client::addResponse(listRes *reslst) {
    Response *res = new Response();
    if (res == NULL) {
        ::std::cerr << "Cannot allocate memory for Response" << ::std::endl;
        return 1;
    }
    reslst->push_back(res);
    return 0;
}

void
Client::fillInResponse(Response *res, Request *req) {
    res->setBody(req->getBody());
    res->formed(true);
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

void
Client::logs(int fd, std::string str, bool isClosed) {
    if (!checkDeepLogs(fd))
        return ;
    if (getUnlink())
        return ;
    if (fd == _fd_cli && logOnlyCliCommands(fd, str, isClosed))
        return;
    logBlind(fd, str, isClosed);
}

bool
Client::checkDeepLogs(int fd) {
    if (g_deep_logs == 0 || 
        (g_deep_logs == 1 && fd != _fd_cli) ||
        (g_deep_logs == 4 && fd != _fd_cli) ||
        (g_deep_logs == 2 && fd != _fd_db))
        return 0;
    return 1;
}

void
Client::logHostPortFd(int fd, bool isClosed) {
    *g_ofs_log << std::left << std::setw(13) << std::setfill(' ') << std::setiosflags (std::ios::showbase)
              << (fd == _fd_cli ? "Client" : "Postgres") 
              << (fd == _fd_cli ? "  from IP:port[ " : "  to   IP:port[ " )
              << std::left << std::setw(21) << std::setfill(' ') << std::setiosflags (std::ios::showbase)
              << _clientHostPort << "] fd[" << fd << "]  " << (isClosed ? "" : "datas: ")
              << std::resetiosflags(std::ios::showbase);
}

bool   
Client::logOnlyCliCommands(int fd, std::string &str, bool isClosed) {
    // std::string msg("RK3CGHWDIEVvnNAtS1sZT dc B2CfDEHFpPpQppSX");
    // size_t id = msg.find_first_of(str[0]);
    if (g_deep_logs != 4)
        return 0;
    // if (id != msg.npos) {
    if (str[0] == 'Q') {
        logTime(g_ofs_log);
        logHostPortFd(fd, isClosed);
        parse(str);
    }
    return 1;
}

void
Client::logBlind(int fd, std::string &str, bool isClosed) {
    logTime(g_ofs_log);
    logHostPortFd(fd, isClosed);
    if (isClosed) {
        *g_ofs_log << str << std::endl;
        return ;
    }

    auto it = str.cbegin();
    std::string msg;
    if (fd == _fd_db)
        msg = "RK3CGHWDIEVvnNAtS1sZTdc";
    else
        msg = "dcB2CfDEHFpPQSX";
    size_t id = msg.find_first_of(str[0]);
    int id_pos = 0;
    if (id != msg.npos) {
        *g_ofs_log << *it ;
        ++it;
        ++id_pos;
    }
    for (int i = 1; (i + id_pos) < 3 && it != str.cend(); ++i) {
        *g_ofs_log << " [" << g_color;
        for (unsigned char c = *it; it != str.cend() && it != str.cbegin() + 4 * i + id_pos; ++it, c = *it) {
            *g_ofs_log << " " << (unsigned int)c << " ";
        }
        *g_ofs_log << "] " << g_color_end;
    }

    for (unsigned char c = *it; it != str.cend(); ++it, c = *it) {
        if (!isprint(c)) {
            *g_ofs_log << " [" << g_color;
            for (; !isprint(c) && it != str.cend(); ++it, c = *it)
                *g_ofs_log << " " << (unsigned int)c << " ";
            *g_ofs_log << "] " << g_color_end;
            if (it == str.cend())
                break ;
        }
        *g_ofs_log << (unsigned char)c ;
    }
    *g_ofs_log << ::std::endl;
}



// public:



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

// test
// #include <algorithm>
//
// void print_code_char(int fd, std::string str) {
//
//     ::std::cout << "makeResponse fd " << fd << "  |" ; // test
//
//     // int len;
//     // char *s = NULL;
//     // sscanf(str.c_str(), "%d%s", &len, s);
//
//     // std::cout << "\n len: " << len << "\n s: " << s << std::endl;
//     std::for_each(str.cbegin(), str.cend(),
//                    [](unsigned char c) { 
//                     if (isprint(c))
//                         ::std::cout << (unsigned char)c ;
//                     else
//                         ::std::cout << "\033[32m" << " " << (unsigned int)c << " " << "\033[0m";
//                      });
// ::std::cout << ::std::endl << ::std::endl;
// }

void Client::tryReceiveData(int fd) {

    listReq *req = (fd == _fd_cli ? &_requests_cli : &_requests_db);    
    addRequest(req);
    receive(req->back(), fd);
    // print_code_char(fd, req->back()->getBody());
    logs(fd, req->back()->getBody(), 0);
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
        logs(fd, "peer closed connection", true);
        setUnlink(true);
        return ;
    }

    if (static_cast<std::size_t>(bytes) >= res->getBody().length() && !res->sent())
        res->sent(true);
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
