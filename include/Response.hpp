#pragma once

#include "Request.hpp"

class Client;

class Response {
private:
    std::string _body;
    size_t      _bodyPos;
    bool        _formed;
    bool        _sent;

public:
    Response();
    ~Response();

    void    setBodyPos(size_t pos);
    void    setBody(std::string body);

    size_t      getBodyPos(void)  const;
    std::string getBody(void)     const;

    bool    formed(void) const ;
    void    formed(bool formed);

    bool    sent(void) const;
    void    sent(bool sent);

    void    clear(void);
};

