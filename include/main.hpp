#pragma once

#include <iostream>
#include <iterator>
#include <string>
#include <cctype>  // isdigit()
#include <fstream>

#include "Server.hpp"
#include "utils.hpp"
#include "config.hpp"

// design:
#define COLOR "\033[32m"
#define COLOR_END "\033[0m"
std::string g_color = COLOR;
std::string g_color_end = COLOR_END;

// logs
# define LOG_PATH "logs"
std::ostream *g_ofs_log;
std::ostream  stdcout(std::cout.rdbuf());
std::ofstream ofs_log;

int g_deep_logs = 3;
int g_output = 0;

//  PostgeSQL server
std::string g_db_host = "127.0.0.1";
std::string g_db_port = "5432";

// server
Server        g_server;


// main.cpp functions
int    checkArgumens(int ac, char **av);
int    check_host_or_port(char* h_or_p);
int    checkPort(std::string port);
int    checkHost(std::string host);

// config
void   config(void);

// logs
void   openLogFile(void);
void   closeLogFile(void);
