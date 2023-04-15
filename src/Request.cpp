#include "Request.hpp"

Request::Request() {
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