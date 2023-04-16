#include "Request.hpp"

Request::Request(Client *client) 
                    : _expBodySize(-1)
                    , _realBodySize(0) {
    setClient(client);
}

Request::~Request() {
}

bool
Request::formed(void) const {
    return _formed;
}

void
Request::formed(bool formed) {
    _formed = formed;
}

int64_t
Request::getExpBodySize(void) const {
    return _expBodySize;
}

void
Request::setExpBodySize(int64_t size) {
    _expBodySize = size;
}

int64_t
Request::getRealBodySize(void) const {
    return _realBodySize;
}

void
Request::setRealBodySize(int64_t size) {
    _realBodySize = size;
}

bool
Request::parseLine(std::string &line) {

    _body += line;
    setRealBodySize(getRealBodySize() + line.length());

    formed(true);

    return formed();
}

Client *
Request::getClient(void) {
    return _client;
}

void
Request::setClient(Client *client) {
    _client = client;
}

