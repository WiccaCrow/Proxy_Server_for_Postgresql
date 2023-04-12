#include "Server.hpp"

Server::Server(void) : fd_listen(-1) {
    event_server.data.fd = -1;

    event_server.data.fd = listenSockfd();
    event_server.events = EPOLLIN;

    fd_epoll  =  epoll_create1(EPOLL_CLOEXEC);
    check_error(fd_epoll == -1,
                "epoll_create1 not created", 1);

    check_error(epoll_ctl(fd_epoll, EPOLL_CTL_ADD, event_server.data.fd, &event_server),
                "epoll_ctl", 1);
    
    events.reserve(MAX_WAIT_CONNECTION); // exception !!! может быть исключение. Обработать в дальнейшем .. внимание
}

Server::~Server(void) {
    clear();
    std::cout << "destr\n";
}

void
Server::clear() {
    if (event_server.data.fd != -1)
        close(event_server.data.fd);
    if (fd_listen != -1)
        close(fd_listen);
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
            std::cerr << "Info: socket -1. Tying next struct addrinfo..." << std::endl;
            continue;
        }
        check_error(setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1,
                    "setsockopt", 1);
    // bind();
        if (bind(fd_listen, p->ai_addr, p->ai_addrlen) == -1) {
            close(fd_listen);
            std::cerr << "Info: bind -1. Tying next struct addrinfo..." << std::endl;
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
    std::cerr << COLOR_ERROR << "Error: " << error_msg << COLOR_END << std::endl;
    clear();
    ::exit(exit_code);
}

void
Server::start(void) {
    int nb_fds;

    while (true) {
        nb_fds = epoll_wait(fd_epoll, events.data(), MAX_EVENTS, -1);
    // check_error(1, "TEST", 2);
    }

}