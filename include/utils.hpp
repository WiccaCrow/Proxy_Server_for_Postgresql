#pragma once

#include <unistd.h>     // fcntl(), close()
#include <fcntl.h>      // fcntl()

int     fd_nonblock(int fd);