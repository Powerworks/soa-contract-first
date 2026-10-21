#ifndef ORDER_SERVICE_H
#define ORDER_SERVICE_H

#include "http_mini.h"

void order_handler(const HttpRequest *req, int client_fd);

#endif
