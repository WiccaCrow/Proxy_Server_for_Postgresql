#include "Server.hpp"


// Constructors and destructor:


Server::Server(void) : _working(true)
                     , _fd_listen(-1)
                     , _fd_epoll(-1) {
    _event.data.fd = listenSockfd();
    _event.events = EPOLLIN;

    _fd_epoll  =  epoll_create1(EPOLL_CLOEXEC);
    checkError(_fd_epoll == -1,
                "epoll_create1 not created", 1);

    checkError(epoll_ctl(_fd_epoll, EPOLL_CTL_ADD, _event.data.fd, &_event),
                "epoll_ctl", 1);
    
    _events.reserve(MAX_WAIT_CONNECTION);
}

Server::~Server(void) {
    clear();
    for (iter_cv it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it != NULL) {
            delete *it;
        }
    }
}



// private:



int
Server::listenSockfd(void) {
    _event.data.fd = -1;

    // for getaddrinfo():
    struct addrinfo *p, *servinfo, hints;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // my IP

    int er_getadrinfo = getaddrinfo(NULL, PORT, &hints, &servinfo);
    checkError(er_getadrinfo,
                gai_strerror(er_getadrinfo), 1);

    // for socket();
    int optval = 1;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        _fd_listen = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if ( _fd_listen == -1) {
            ::std::cerr << "Info: socket -1. Tying next struct addrinfo..." << ::std::endl;
            continue;
        }
        checkError(setsockopt(_fd_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1,
                    "setsockopt", 1);
    // for bind();
        if (bind(_fd_listen, p->ai_addr, p->ai_addrlen) == -1) {
            close(_fd_listen);
            ::std::cerr << "Info: bind -1. Tying next struct addrinfo..." << ::std::endl;
            continue;
        }
        break;
    }

    checkError(p == NULL,
                "cannot bind", 1);
    
    freeaddrinfo(servinfo);

    // for listen();
    checkError(listen(_fd_listen, MAX_WAIT_CONNECTION) == -1,
                "listen", 1 );

    return _fd_listen;
}



// protected:



void
Server::clear() {
    if (_fd_listen != -1)
        close(_fd_listen);
    if (_event.data.fd != -1)
        close(_event.data.fd);
    if (_fd_epoll != -1)
        close(_fd_epoll);
}

int
Server::checkError(bool is_error, std::string error_msg, int exit_code) {
    if (is_error)
        ::std::cerr << (exit_code ? COLOR_ERROR_FATAL : COLOR_ERROR) << "Error: " << error_msg << COLOR_END << ::std::endl;

    if (is_error && exit_code) {
        clear();
        ::exit(exit_code);
    }
    return is_error;
}

void
Server::epollerr(int i) {
    checkError(1, "epoll: EPOLLERR", 0);
    checkError(_events[i].data.fd == _fd_listen, 
                "fatal: EPOLLERR on listening socket", 1);
    close(_events[i].data.fd);
    getClient(i)->setUnlink(true);
}

void
Server::epollhup(int i) {
    checkError(1, "epoll: EPOLLHUP", 0);
    checkError(_events[i].data.fd == _fd_listen, 
                "fatal: EPOLLHUP on listening socket", 1);
    close(_events[i].data.fd);
    getClient(i)->setUnlink(true);
}

void
Server::epollin(int i) {
    int fd = _events[i].data.fd;
    if(fd == _fd_listen) {
        addNewClient(i);
        return ;
    }

    Client *client = getClient(i);
    if (client == NULL || client->getUnlink())
        return;

    client->tryReceiveData(fd);
    if (client->getUnlink())
        return;

    client->makeResponse(fd);
}

void
Server::addNewClient(int i_events) {

    struct sockaddr_in clientData;
    int fd_cli = fdClient(clientData);
    if (fd_cli == -1)
        return ;

    int fd_db  = fdDatabase();
    if (fd_db == -1) {
        close(fd_cli);
        return ;
    }

    ::Client *client = new ::Client(clientIpPort(clientData), fd_cli, fd_db, i_events);
    if (client == NULL) {
        close(fd_cli);
        if (fd_db != fd_cli)
            close(fd_db);
        return ;
    }

    addClient(fd_cli, fd_db, client);

    if (addFdsToEpoll(fd_cli, client))
        return ;

    if (addFdsToEpoll(fd_db,  client))
        epoll_ctl(_fd_epoll, EPOLL_CTL_DEL, fd_cli, NULL);
}

int    
Server::fdClient(struct sockaddr_in &clientData) {

    socklen_t          clientLen = sizeof(clientData);
    
    int fd = accept(_fd_listen, (struct sockaddr *)&clientData, &clientLen);
    if (fd < 0) {
        checkError(1, " not fatal: Server::accept error", 0);
        return -1;
    }
    if (fd_nonblock(fd)) {
        checkError(1, " not fatal: Server::nonblock error(fcntl). Client was not added.", 0);
        return -1;
    }
    return fd;
}

int 
Server::fdDatabase(void) {

    struct addrinfo *addrlst = NULL;

    if (getaddrinfo(g_db_host.c_str(), g_db_port.c_str(), NULL, &addrlst)) {
        checkError(1, "Proxy::getaddrinfo -> " + g_db_host + ":" + g_db_port, 1);
        return -1;
    }

    int fd = setConnectionDb(addrlst);
    freeaddrinfo(addrlst);

    return fd;
}

int
Server::setConnectionDb(struct addrinfo *lst) {

    // get socket
    int fd = ::socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        checkError(fd < 0, "Server: db connection: Cannot create socket", 0);
        return -1;
    }

    // try to connect
    int connected = 1;
    for (; lst && connected; lst = lst->ai_next) {
        if (lst->ai_socktype != SOCK_STREAM) {
            continue;
        }
        connected = ::connect(fd, lst->ai_addr, lst->ai_addrlen);
    }

    // set nonblock
    if (checkError(connected == -1, 
                    "Server: db connection: connection failed", 0) ||
        checkError(fd_nonblock(fd) < 0, 
                    "Server: db connection: fcntl failed ", 0)) {
        return -1;
    }
    // std::cout << "connaction established" << std::endl; // test
    return fd;
}

std::string 
Server::clientIpPort(struct sockaddr_in &clientData) {
    std::string str = inet_ntoa(clientData.sin_addr);
    str += ':';
    str += std::to_string(ntohs(clientData.sin_port));
    return (str);
}

static int
isClientFree(::Client *client) {
    return (client == NULL);
}

void
Server::addClient(int fd_cli, int fd_db, ::Client *client) {

    iter_cv it = ::std::find_if(_clients.begin(), _clients.end(), isClientFree);
    if (it == _clients.end()) {
        it = _clients.insert(_clients.end(), client);
    }
     
    *it = client;

    size_t id = std::distance(_clients.begin(), it);
    client->setId(id);

    _connector[fd_cli] = _connector[fd_db] = client->getId();
}

void
Server::rmClient(::Client *client) {
    _clients[client->getId()] = NULL;
    _connector.erase(client->getFdCli());
    _connector.erase(client->getFdDb());
    delete client;
}

int
Server::addFdsToEpoll(int fd, Client *client) {
    _event.data.fd = fd;
    _event.events = EPOLLIN | EPOLLOUT;
    int epoll_result = epoll_ctl(_fd_epoll, EPOLL_CTL_ADD, fd, &_event);

    int ret = checkError( epoll_result == -1,
                "Server: epoll_ctl for new client", 0);
    if (ret) {
        rmClient(client);
        close(client->getFdCli());
        close(client->getFdDb());
    }
    return ret;
}

void
Server::epollout(int i) {
    int fd = _events[i].data.fd;
    (void)fd;

    Client *client = getClient(i);
    if (client == NULL || client->getUnlink())
        return;

    client->tryReplyData(fd);
}

Client*
Server::getClient(int i) {
    int fd = _events[i].data.fd;
    if(fd == _fd_listen)
        return NULL;

    size_t id = _connector[fd];
    if (id >= _clients.size()) {
        ::std::cout << "Server::start:: invalid clients id: " << fd << " " << id << ::std::endl;
        return NULL;
    }

    Client *client = _clients[id];
    if (client == NULL)
        return NULL ;

    return client;
}



// public:



static void
sigint_handler(int) {
    std::cout << std::endl;
    ::std::cout << "Server is stopping CTRL+C" << ::std::endl;
    g_server.finish();
}

void
Server::start(void) {
    int nb_fds;
    int wait_connection = MAX_WAIT_CONNECTION;
    signal(SIGINT, sigint_handler);
    while (_working && (nb_fds = epoll_wait(_fd_epoll, _events.data(), MAX_EVENTS, -1)) != -1) {
    
        for(int i = 0; i < nb_fds; ++i) {
            if(_events[i].events & EPOLLERR) {
                epollerr(i);
            } else if(_events[i].events & EPOLLHUP) {
                epollhup(i);
            } else if (_events[i].events & EPOLLIN) {
                epollin(i);
            } else if (_events[i].events & EPOLLOUT) {
                epollout(i);
            } else {
                ::std::cout << "INFO: Server: start: epoll events other" << ::std::endl;
            }
            // EPOLLPRI 
            unlink(getClient(i));
        }
        if (wait_connection == nb_fds) {
            wait_connection *= 2;
            _events.reserve(wait_connection); // exception !!! может быть исключение. Обработать в дальнейшем .. внимание
        }
    }
    if (_working)
        checkError(nb_fds == -1,
                " fatal: Server: epoll_wait -1", 1);
}

void
Server::unlink(Client *cli) {
    if (cli == NULL)
        return ;
    if (!cli->getUnlink())
        return ;
    int fd_cli = cli->getFdCli();
    int id = _connector[fd_cli];

    if (id < 0) {
        return ;
    }

    if (fd_cli >= 0) {
        close(fd_cli);
    }

    int fd_db = cli->getFdDb();
    if (fd_db >= 0) {
        close(fd_db);
    }

    epoll_ctl(_fd_epoll, EPOLL_CTL_DEL, fd_cli, NULL);
    epoll_ctl(_fd_epoll, EPOLL_CTL_DEL, fd_db, NULL);
    rmClient(cli);
}

void 
Server::finish(void) {
    _working = false;
}
