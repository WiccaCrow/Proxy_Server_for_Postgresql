#pragma once

#include <unistd.h>     // fcntl(), close()
#include <fcntl.h>      // fcntl()

class Client {
    public:
    explicit Client(int fdread = -1, int fdwrite = -1);
            ~Client(void);

    int     nonblock(void);
    void    set_id(int id_cli);
    void    set_fd_read(int fd);
    void    set_fd_write(int fd);

    private:
    int     fd_read;
    int     fd_write;
    int     id;
};