#include "Server.hpp"

Server::Server(void) :  _fd_listen(-1),
                        _fd_epoll(-1) {
    _event.data.fd = -1;

    _event.data.fd = listenSockfd();
    _event.events = EPOLLIN;

    _fd_epoll  =  epoll_create1(EPOLL_CLOEXEC);
    check_error(_fd_epoll == -1,
                "epoll_create1 not created", 1);

    check_error(epoll_ctl(_fd_epoll, EPOLL_CTL_ADD, _event.data.fd, &_event),
                "epoll_ctl", 1);
    
    _events.reserve(MAX_WAIT_CONNECTION); // exception !!! может быть исключение. Обработать в дальнейшем .. внимание
}

Server::~Server(void) {
    clear();
    for (iter_cv it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it != NULL) {
            delete *it;
        }
    }
    ::std::cout << "destr\n";
}

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
Server::listenSockfd(void) {

    // for getaddrinfo():
    struct addrinfo *p, *servinfo, hints;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // my IP

    int er_getadrinfo = getaddrinfo(NULL, PORT, &hints, &servinfo);
    check_error(er_getadrinfo,
                gai_strerror(er_getadrinfo), 1);

    // for socket();
    int optval = 1;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        _fd_listen = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if ( _fd_listen == -1) {
            ::std::cerr << "Info: socket -1. Tying next struct addrinfo..." << ::std::endl;
            continue;
        }
        check_error(setsockopt(_fd_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1,
                    "setsockopt", 1);
    // for bind();
        if (bind(_fd_listen, p->ai_addr, p->ai_addrlen) == -1) {
            close(_fd_listen);
            ::std::cerr << "Info: bind -1. Tying next struct addrinfo..." << ::std::endl;
            continue;
        }
        break;
    }

    check_error(p == NULL,
                "cannot bind", 1);
    
    freeaddrinfo(servinfo);

    // for listen();
    check_error(listen(_fd_listen, MAX_WAIT_CONNECTION) == -1,
                "listen", 1 );

    return _fd_listen;
}

int
Server::check_error(bool is_error, std::string error_msg, int exit_code) {
    if (is_error)
        ::std::cerr << (exit_code ? COLOR_ERROR_FATAL : COLOR_ERROR) << "Error: " << error_msg << COLOR_END << ::std::endl;

    if (is_error && exit_code) {
        clear();
        ::exit(exit_code);
    }
    return is_error;
}

void
Server::start(void) {
    // check_error(1, "TEST", 2);
    int nb_fds;
    int wait_connection = MAX_WAIT_CONNECTION;
    while ((nb_fds = epoll_wait(_fd_epoll, _events.data(), MAX_EVENTS, -1)) != -1) {

        for(int i = 0; i < nb_fds; ++i) {
            if(_events[i].events & EPOLLERR) {
                epollerr(i);
                continue;
            } else if(_events[i].events & EPOLLHUP) {
                epollhup(i);
                continue;
            } else if (_events[i].events & EPOLLIN) {
                epollin(i);
            } else {
                ::std::cout << "INFO: Server: start: epoll events other" << ::std::endl;
            }
        }
        if (wait_connection == nb_fds) {
            wait_connection *= 2;
            _events.reserve(wait_connection); // exception !!! может быть исключение. Обработать в дальнейшем .. внимание
        }
    }
    check_error(nb_fds == -1,
                " fatal: Server: epoll_wait -1", 1);
}

void
Server::epollerr(int i) {
    ::std::cout << " epollerr client i = " << i << ::std::endl;
    check_error(1, "epoll: EPOLLERR", 0);
    check_error(_events[i].data.fd == _fd_listen, 
                "fatal: EPOLLERR on listening socket", 1);
    close(_events[i].data.fd);
    _events[i].data.fd = -1;
}

void
Server::epollhup(int i) {
    ::std::cout << " epollhup client i = " << i << ::std::endl;
    check_error(1, "epoll: EPOLLHUP", 0);
    check_error(_events[i].data.fd == _fd_listen, 
                "fatal: EPOLLHUP on listening socket", 1);
    close(_events[i].data.fd);
    _events[i].data.fd = -1;
}

void
Server::epollin(int i) {
    int fd = _events[i].data.fd;
    if(fd == _fd_listen) {
        // ::std::cout << "Server::pollin:: new test" << ::std::endl;
        add_new_client(i);
    } else {
        // найти клиента
        // ::std::cout << "Server::pollin:: read test" << ::std::endl;
        size_t id = _connector[fd];

        if (id >= _clients.size()) {
            ::std::cout << "Server::pollin:: invalid id: " << fd << " " << id << ::std::endl;
            return ;
        }

        Client *client = _clients[id];
        if (client == NULL) {
            ::std::cout << "Server::pollin:: client for " << id << ": NULL " << ::std::endl;
            return ;
        }

        // получение данных от клиента
        if (fd == client->get_fd_cli()) {
            client->tryReceiveRequest(client->get_fd_cli());
        } else {
            client->tryReceiveRequest(client->get_fd_db());
        }


        // вызвать чтение - простой тест
        // char buf[20000] = {0};
        // read(fd, buf, 2000);
        // ::std::cout << buf << std::endl;
    }
         
}

void
Server::add_new_client(int i_events) {

    int fd_cli = fd_client();
    if (fd_cli == -1)
        return ;

    int fd_db  = fd_database();
    if (fd_db == -1) {
        close(fd_cli);
        return ;
    }

    ::Client *client = new ::Client(fd_cli, fd_db, i_events);
    if (client == NULL) {
        ::std::cout << "Error: not fatal: Server::Cannot allocate memory for Client" << ::std::endl;
        close(fd_cli);
        if (fd_db != fd_cli)
            close(fd_db);
        return ;
    }

    addClient(fd_cli, fd_db, client);
    // ::std::cout << "Info: Server::connect [" << fd << "] " << ::std::endl;

    if (addFdsToEpoll(fd_cli, client))
        return ;

    if (addFdsToEpoll(fd_db,  client))
        epoll_ctl(_fd_epoll, EPOLL_CTL_DEL, fd_cli, NULL);
}

int
Server::addFdsToEpoll(int fd, Client *client) {
    _event.data.fd = fd;
    _event.events = EPOLLIN;
    int epoll_result = epoll_ctl(_fd_epoll, EPOLL_CTL_ADD, fd, &_event);

    int ret = check_error( epoll_result == -1,
                "Server: epoll_ctl for new client", 0);
    if (ret) {
        rmClient(client);
        close(client->get_fd_cli());
        close(client->get_fd_db());
    }
    return ret;
}

int 
Server::fd_database(void) {

    struct addrinfo *addrlst = NULL;

    if (getaddrinfo(g_db_host.c_str(), g_db_port.c_str(), NULL, &addrlst)) {
        check_error(1, "Proxy::getaddrinfo -> " + g_db_host + ":" + g_db_port, 1);
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
        check_error(fd < 0, "Server: db connection: Cannot create socket", 0);
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
    if (check_error(connected == -1, 
                    "Server: db connection: connection failed", 0) ||
        check_error(fcntl(fd, F_SETFL, O_NONBLOCK) < 0, 
                    "Server: db connection: fcntl(O_NONBLOCK) failed ", 0)) {
        close(fd);
        return -1;
    }
    // std::cout << "connaction established" << std::endl; // test
    return fd;
}

int    
Server::fd_client(void) {

    struct sockaddr_in clientData;
    socklen_t          clientLen = sizeof(clientData);
    
    int fd = accept(_fd_listen, (struct sockaddr *)&clientData, &clientLen);
    if (fd < 0) {
        check_error(1, " not fatal: Server::accept error", 0);
        return -1;
    }
    if (fd_nonblock(fd)) {
        check_error(1, " not fatal: Server::nonblock error(fcntl). Client was not added.", 0);
        close(fd);
        return -1;
    }
    return fd;
}

static int
isClientFree(::Client *client) {
    return (client == NULL);
}

void
Server::rmClient(::Client *client) {

    _clients[client->get_id()] = NULL;
    delete client;

    _connector.erase(client->get_fd_cli());
    _connector.erase(client->get_fd_db());

}

void
Server::addClient(int fd_cli, int fd_db, ::Client *client) {

    iter_cv it = ::std::find_if(_clients.begin(), _clients.end(), isClientFree);
    if (it == _clients.end()) {
        it = _clients.insert(_clients.end(), client);
    }
     
    *it = client;

    size_t id = std::distance(_clients.begin(), it);
    client->set_id(id); // по id можно найти клиента к удалению при закрытии fd

    _connector[fd_cli] = _connector[fd_db] = client->get_id();
}

void
Server::unlink(Client *cli) {
    int fd_cli = cli->get_fd_cli();
    int id = _connector[fd_cli];

    if (id < 0) {
        return ;
    }

    if (fd_cli >= 0) {
        close(fd_cli);
    }

    int fd_db = cli->get_fd_db();
    if (fd_db >= 0) {
        close(fd_db);
    }

    epoll_ctl(_fd_epoll, EPOLL_CTL_DEL, fd_cli, NULL);
    epoll_ctl(_fd_epoll, EPOLL_CTL_DEL, fd_db, NULL);
    rmClient(cli);
}
