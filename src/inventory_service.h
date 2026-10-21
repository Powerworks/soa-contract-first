#ifndef INVENTORY_SERVICE_H
#define INVENTORY_SERVICE_H

#include "http_mini.h"

void inventory_handler(const HttpRequest *req, int client_fd);

#endif
