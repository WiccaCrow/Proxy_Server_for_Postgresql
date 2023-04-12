#include "Client.hpp"

Client::Client(int fdread, int fdwrite) : 
                fd_read(fdread),
                fd_write(fdwrite),
                id(-1) {

}

Client::~Client(void) {
    if (fd_read != -1) {
        if (fd_read == fd_write) {
            fd_write = -1;
        }
        close(fd_read);
        fd_read = -1;
    }
    if (fd_write != -1) {
        close(fd_write);
    }

}

void
Client::set_id(int id_cli) {
    id = id_cli;
}

void
Client::set_fd_read(int fd) {
    fd_read = fd;
}

void
Client::set_fd_write(int fd) {
    fd_write = fd;
}