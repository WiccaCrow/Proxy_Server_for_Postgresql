#pragma once

#include <cstdint> // int64_t
#include <string>  // arg in parseLine()

class Client;

class Request {
private:
    bool        _formed;
    int64_t     _expBodySize;
    int64_t     _realBodySize;
    std::string _body;
    Client    * _client;

public:
    Request(Client *client);
    ~Request(void);

    bool    formed(void) const ;
    void    formed(bool formed);

    void    setExpBodySize(int64_t);
    void    setRealBodySize(int64_t);

    int64_t getExpBodySize(void) const;
    int64_t getRealBodySize(void) const;

    Client *getClient(void);
    void    setClient(Client *);

    bool parseLine(std::string &);

};
