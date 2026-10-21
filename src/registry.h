#ifndef REGISTRY_H
#define REGISTRY_H

#include "http_mini.h"

/* Tiny in-memory service registry: name -> host:port.
 * Services POST /register with {name, address}; consumers GET /lookup?name=X */
void registry_handler(const HttpRequest *req, int client_fd);

#endif
