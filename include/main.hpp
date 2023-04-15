#pragma once

#include <iostream>
#include <iterator>
#include <string>
#include <cctype> // isdigit()

#include "Server.hpp"

//  PostgeSQL server
std::string g_db_host = "127.0.0.1";
std::string g_db_port = "5432";

int    check_ac_av(int ac, char **av);
int    check_host_or_port(char* h_or_p);
int     check_port(std::string port);
int    check_host(std::string host);