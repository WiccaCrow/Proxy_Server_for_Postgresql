#include "Response.hpp"

Response::Response(void)
                : _bodyPos(0)
                , _formed(false)
                , _sent(false) {
}

Response::~Response() {
}

std::string
Response::getBody(void) const {
    return _body;
}

size_t
Response::getBodyPos(void) const {
    return _bodyPos;
}

void
Response::setBody(std::string body) {
    _body = body;
}

void
Response::setBodyPos(size_t pos) {
    _bodyPos = pos;
}

bool
Response::formed(void) const {
    return _formed;
}

void
Response::formed(bool formed) {
    _formed = formed;
}

bool
Response::sent(void) const {
    return _sent;
}

void
Response::sent(bool sent) {
    _sent = sent;
}

void
Response::clear(void) {
    _body = "";
    _bodyPos = 0;
}