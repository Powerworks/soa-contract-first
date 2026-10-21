#include "http_mini.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile int g_server_fd = -1;

static void handle_sigint(int sig) {
    (void)sig;
    if (g_server_fd >= 0) close(g_server_fd);
    exit(0);
}

static bool read_line(int fd, char *buf, size_t len) {
    size_t i = 0;
    char c;
    while (i + 1 < len && read(fd, &c, 1) == 1) {
        if (c == '\n') {
            buf[i] = '\0';
            return true;
        }
        if (c != '\r') buf[i++] = c;
    }
    buf[i] = '\0';
    return i > 0;
}

void http_server_run(int port, HttpHandler handler) {
    struct sockaddr_in addr;
    int opt = 1;

    g_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_server_fd < 0) { perror("socket"); exit(1); }

    setsockopt(g_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(g_server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(g_server_fd, 16) < 0) { perror("listen"); exit(1); }

    signal(SIGINT, handle_sigint);

    while (1) {
        int client_fd = accept(g_server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        HttpRequest req;
        memset(&req, 0, sizeof(req));
        req.client_fd = client_fd;

        char line[1024];
        if (!read_line(client_fd, line, sizeof(line))) { close(client_fd); continue; }

        /* Parse request line: METHOD PATH HTTP/1.1 */
        sscanf(line, "%15s %255s", req.method, req.path);

        /* Skip headers */
        while (read_line(client_fd, line, sizeof(line)) && strcmp(line, "") != 0) {}

        /* Read body if Content-Length present (simplified) */
        req.body[0] = '\0';

        handler(&req, client_fd);
        close(client_fd);
    }
}

void http_send_response(int client_fd, int status, const char *status_text,
                        const char *content_type, const char *body) {
    char header[1024];
    int hlen = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, status_text, content_type, body ? strlen(body) : 0);
    write(client_fd, header, hlen);
    if (body) write(client_fd, body, strlen(body));
}

void http_send_json(int client_fd, int status, const char *json) {
    http_send_response(client_fd, status, "OK", "application/json", json);
}
