#include "Response.hpp"


// Constructors and destructor:


Response::Response(void)
                : _bodyPos(0)
                , _formed(false)
                , _sent(false) {
}

Response::~Response() {
}



// public:



void
Response::setBodyPos(size_t pos) {
    _bodyPos = pos;
}

void
Response::setBody(std::string body) {
    _body = body;
}

size_t
Response::getBodyPos(void) const {
    return _bodyPos;
}

std::string
Response::getBody(void) const {
    return _body;
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
