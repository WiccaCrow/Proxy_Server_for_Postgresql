#include "utils.hpp"
#include <iostream>     // cout, cerr

int
fd_nonblock(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        ::std::cout << "\033[35m" << "fcntl(O_NONBLOCK) failed [" << fd << "]" << "\033[0m" << std::endl;
        close(fd);
        return -1;
    }
    return 0;
}

void outWithFlags(int nb, std::string str, std::ostream *ofs) {
    *ofs << std::right << std::setw(2) << std::setfill('0') << std::setiosflags (std::ios::showbase)
              << nb << str
              << std::resetiosflags(std::ios::showbase);
}

bool    logTime(std::ostream *ofs) {

    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    *ofs << (now->tm_year + 1900) << '.' ;
    outWithFlags(now->tm_mon + 1, ".", ofs);
    outWithFlags(now->tm_mday, " ", ofs);
    outWithFlags(now->tm_hour, ":", ofs);
    outWithFlags(now->tm_min, ":", ofs);
    outWithFlags(now->tm_sec, "  ", ofs);
    return true;
}
