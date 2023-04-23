#pragma once

#include <unistd.h>     // fd_nonblock(), fcntl(), close()
#include <fcntl.h>      // fd_nonblock(), fcntl()
#include <ctime>        // logTime()
#include <chrono>       // logTime()
#include <iomanip>      // logTime()
#include <fstream>      // logTime()

int     fd_nonblock(int fd);
bool    logTime(std::ostream *ofs);
int     convRawBytesToInt32(std::array<int32_t, 4> raw);
int     convRawBytesToInt16(std::array<int16_t, 2> raw);
