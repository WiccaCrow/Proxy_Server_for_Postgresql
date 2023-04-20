#pragma once

#include <unistd.h>     // fd_nonblock(), fcntl(), close()
#include <fcntl.h>      // fd_nonblock(), fcntl()

#include <ctime>        // logTime()
#include <chrono>       // logTime()
#include <iomanip>      // logTime()

#include <fstream>

int     fd_nonblock(int fd);
bool    logTime(std::ostream *ofs);
