#pragma once

#include <cstring> // strcmp()
#include <string>
#include <fstream>
#include <iostream>
#include <string>

#include "globals.hpp"

# define CONFIG_TCPSERV "./TCPserv.conf"

int     configValue(const char * str_conf, const char *msg, size_t length);
void    config(void);

