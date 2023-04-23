#include "utils.hpp"
#include <iostream>     // cout, cerr

int     fd_nonblock(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        ::std::cout << "\033[35m" << "fcntl(O_NONBLOCK) failed [" << fd << "]" << "\033[0m" << std::endl;
        close(fd);
        return -1;
    }
    return 0;
}

void    outWithFlags(int nb, std::string str, std::ostream *ofs) {
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

int    convRawBytesToInt32(std::array<int32_t, 4> raw) {
    return raw[0] << 24 | raw[1] << 16 | raw[2] << 8 | raw[3];
}

int    convRawBytesToInt16(std::array<int16_t, 2> raw) {
    return  raw[0] << 8 | raw[1];
}