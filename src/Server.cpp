#include "Server.hpp"

Server::Server(void) : fd_listen(-1) {
    event.data.fd = -1;

    event.data.fd = listenSockfd();
    event.events = EPOLLIN;

    fd_epoll  =  epoll_create1(EPOLL_CLOEXEC);
    check_error(fd_epoll == -1,
                "epoll_create1 not created", 1);

    check_error(epoll_ctl(fd_epoll, EPOLL_CTL_ADD, event.data.fd, &event),
                "epoll_ctl", 1);
    
    events.reserve(MAX_WAIT_CONNECTION); // exception !!! может быть исключение. Обработать в дальнейшем .. внимание
}

Server::~Server(void) {
    clear();
    ::std::cout << "destr\n";
}

void
Server::clear() {
    if (fd_listen != -1)
        close(fd_listen);
    if (event.data.fd != -1)
        close(event.data.fd);
}

int
Server::listenSockfd(void) {

    // getaddrinfo():
    struct addrinfo *p, *servinfo, hints;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // my IP

    int er_getadrinfo = getaddrinfo(NULL, PORT, &hints, &servinfo);
    check_error(er_getadrinfo,
                gai_strerror(er_getadrinfo), 1);
    // socket();
    int optval = 1;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        fd_listen = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if ( fd_listen == -1) {
            ::std::cerr << "Info: socket -1. Tying next struct addrinfo..." << ::std::endl;
            continue;
        }
        check_error(setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1,
                    "setsockopt", 1);
    // bind();
        if (bind(fd_listen, p->ai_addr, p->ai_addrlen) == -1) {
            close(fd_listen);
            ::std::cerr << "Info: bind -1. Tying next struct addrinfo..." << ::std::endl;
            continue;
        }
        break;
    }

    check_error(p == NULL,
                "cannot bind", 1);
    
    freeaddrinfo(servinfo);

    // listen();
    check_error(listen(fd_listen, MAX_WAIT_CONNECTION) == -1,
                "listen", 1 );

    return fd_listen;
}

void
Server::check_error(bool check, std::string error_msg, int exit_code) {
    if (check)
        exit(error_msg, exit_code);
}

void
Server::exit(std::string error_msg, int exit_code) {
    ::std::cerr << COLOR_ERROR_FATAL << "Error: " << error_msg << COLOR_END << ::std::endl;
    clear();
    ::exit(exit_code);
}

void
Server::start(void) {
    // check_error(1, "TEST", 2);
    int nb_fds;

    while (true) {
        
        nb_fds = epoll_wait(fd_epoll, events.data(), MAX_EVENTS, -1);

        for(int i = 0; i < nb_fds; ++i)
        {
            if(events[i].events & EPOLLERR) {
                epollerr(i);
                continue;
            } else if(events[i].events & EPOLLHUP) {
                epollhup(i);
                continue;
            } else if (events[i].events & EPOLLIN) {
                epollin(i);
            } else {
                ::std::cout << "INFO: Server: start: epoll events other" << ::std::endl;
            }    
        }
    }
}

void
Server::epollerr(int i) {
    ::std::cout << " clietn i = " << i << ::std::endl;
    epoll_err_msg("EPOLLERR");
    check_error(events[i].data.fd == fd_listen, 
                "fatal: EPOLLERR on listening socket", 1);
    close(events[i].data.fd);
    events[i].data.fd = -1;
}

void
Server::epollhup(int i) {
    ::std::cout << " clietn i = " << i << ::std::endl;
    epoll_err_msg("EPOLLHUP");
    check_error(events[i].data.fd == fd_listen, 
                "fatal: EPOLLHUP on listening socket", 1);
    close(events[i].data.fd);
    events[i].data.fd = -1;
}

void
Server::epoll_err_msg(::std::string msg) {
    ::std::cerr << COLOR_ERROR_EPOL << "Error: epoll " << msg << COLOR_END << ::std::endl;

}

void
Server::epollin(int i) {
    if(events[i].data.fd == fd_listen) {
        add_new_client();
    } else {
        // read write
    }
}

void
Server::add_new_client(void) {

    struct sockaddr_in clientData;
    socklen_t          clientLen = sizeof(clientData);
    
    int fd = accept(fd_listen, (struct sockaddr *)&clientData, &clientLen);
    if (fd < 0) {
        ::std::cout << "Error: not fatal: Server::accept" << ::std::endl;
        return;
    }

    ::Client *client = new ::Client(fd, fd);
    if (client == NULL) {
        ::std::cout << "Error: not fatal: Server::Cannot allocate memory for Client" << ::std::endl;
        close(fd);
        return ;
    }

    // struct hostent *he = NULL;
    // he = gethostbyaddr(&clientData.sin_addr, sizeof(clientData.sin_addr), AF_INET);
    // внимание . Для прокси возможно, вернусь

    if (fd_nonblock(fd)) {
        ::std::cout << "Error: not fatal: Server::nonblock error(fcntl). Client was not added." << ::std::endl;
        close(fd);
        return;
    }

    addClient(client);
    // ::std::cout << "Info: Server::connect [" << fd << "] " << ::std::endl;

    event.data.fd = fd;
    event.events = EPOLLIN;
    check_error(epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd, &event) == -1,
                "fatal: epoll_ctl for new client", 1);
}

static int
isClientFree(::Client *client) {
    return (client == NULL);
}

void
Server::addClient(::Client *client) {

    iter_cv it = ::std::find_if(_clients.begin(), _clients.end(), isClientFree);
    if (it == _clients.end()) {
        it = _clients.insert(_clients.end(), client);
    }
     
    *it = client;

    size_t id = std::distance(_clients.begin(), it);
    client->set_id(id);
}
