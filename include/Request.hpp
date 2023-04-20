#pragma once

#include <cstdint> // int64_t
#include <string>  // arg in parseLine()

#include<iostream> // tests внимание

class Client;

class Request {
    
private:

    std::string _body;
    bool        _formed;
    int64_t     _expBodySize;
    int64_t     _realBodySize;
    bool        _sent;

    Client    * _client;
    
public:
    Request(Client *client);
    ~Request(void);

    bool    formed(void) const ;
    void    formed(bool formed);
   
    void    setExpBodySize(int64_t size);
    void    setRealBodySize(int64_t size);
    void    setBody(std::string body);

    int64_t     getExpBodySize(void)  const;
    int64_t     getRealBodySize(void) const;
    std::string getBody(void)         const;

    Client *getClient(void);
    void    setClient(Client *client);

    bool    parseLine(std::string &line);

    bool    sent(void) const;
    void    sent(bool sent);

    void    clearBody(void);
};
