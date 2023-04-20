#include "Request.hpp"
#include "Client.hpp"


// Constructors and destructor:


Request::Request(Client *client)
                : _formed(false) 
                , _expBodySize(-1)
                , _realBodySize(0)
                , _sent(false) {
    setClient(client);
}

Request::~Request() {
}



// public:



bool
Request::formed(void) const {
    return _formed;
}

void
Request::formed(bool formed) {
    _formed = formed;
}

void
Request::setExpBodySize(int64_t size) {
    _expBodySize = size;
}

void
Request::setRealBodySize(int64_t size) {
    _realBodySize = size;
}

void
Request::setBody(std::string body) {
    _body = body;
}

int64_t
Request::getExpBodySize(void) const {
    return _expBodySize;
}

int64_t
Request::getRealBodySize(void) const {
    return _realBodySize;
}

std::string
Request::getBody(void) const {
    return _body;
}

Client *
Request::getClient(void) {
    return _client;
}

void
Request::setClient(Client *client) {
    _client = client;
}

bool
Request::parseLine(std::string &line) {
    _body += line;
    
    setRealBodySize(getRealBodySize() + line.length());

    formed(true);
    return formed();
}

bool
Request::sent(void) const {
    return _sent;
}

void
Request::sent(bool sent) {
    _sent = sent;
}

void
Request::clearBody(void) {
    _body = "";
}

