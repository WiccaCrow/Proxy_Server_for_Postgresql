#include "Request.hpp"
#include "Client.hpp"

Request::Request(Client *client) {
    setClient(client);
    
    // _bodyPos = 0;
    _formed = false;
    _expBodySize = -1;
    _realBodySize = 0;
    _sent = false;

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

std::string
Request::getBody(void) const {
    return _body;
}

// size_t
// Request::getBodyPos(void) const {
//     return _bodyPos;
// }

void
Request::setRealBodySize(int64_t size) {
    _realBodySize = size;
}

void
Request::setBody(std::string body) {
    _body = body;
}

// void
// Request::setBodyPos(size_t pos) {
//     _bodyPos = pos;
// }

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
    // _bodyPos = 0;
}

