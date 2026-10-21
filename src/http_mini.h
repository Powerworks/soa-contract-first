#ifndef HTTP_MINI_H
#define HTTP_MINI_H

#include <stddef.h>

typedef struct {
    int  client_fd;
    char method[16];
    char path[256];
    char body[4096];
} HttpRequest;

typedef void (*HttpHandler)(const HttpRequest *req, int client_fd);

/* Start a tiny HTTP/1.1 server on `port`. `handler` is invoked per request. */
void http_server_run(int port, HttpHandler handler);

/* Send an HTTP response with status line, headers, and body. */
void http_send_response(int client_fd, int status, const char *status_text,
                        const char *content_type, const char *body);

/* Send a JSON response. */
void http_send_json(int client_fd, int status, const char *json);

#endif
