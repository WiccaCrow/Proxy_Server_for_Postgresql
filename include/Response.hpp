#pragma once

#include "Request.hpp"

class Client;

class Response {
private:
    Request    *_req;
    Client     * _client;

public:
    Response(Request *req);
    ~Response();

    void    setClient(Client *client);
};

