#include "utils.hpp"
#include <iostream>     // cout, cerr

#define COLOR_ERROR "\033[35m"
#define COLOR_END "\033[0m"

int
fd_nonblock(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        ::std::cout << COLOR_ERROR << "fcntl(O_NONBLOCK) failed [" << fd << "]" << COLOR_END << std::endl;
        return -1;
    }
    return 0;
}