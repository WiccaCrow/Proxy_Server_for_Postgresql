#pragma  once

#include <string>
#include <map>

#include "utils.hpp"
#include "globals.hpp"

typedef void (*Parser)(std::string &str);

void    parse(std::string &str);
void    Close(std::string &str);
void    Query(std::string &str);
void    CopyData(std::string &str);
void    CopyFail(std::string &str);
void    others(std::string &str);

extern const std::map<char, Parser> validReqCli;