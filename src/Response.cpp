#include "Response.hpp"

Response::Response(Request *req)
         : _req(req) {
    setClient(_req->getClient());
}

Response::~Response() {
}

void
Response::setClient(Client *client) {
    _client = client;
}
