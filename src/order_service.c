#include "order_service.h"
#include "contracts.h"
#include "json_mini.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* order-service talks to inventory-service over HTTP. This is the SOA boundary:
 * the order service does not know inventory's internals, only its contract. */

static const char *g_inventory_addr = NULL;

static void post_to_inventory(const char *path, const char *body, char *out, size_t out_size) {
    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { snprintf(out, out_size, "{\"error\":\"socket\"}"); return; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082); /* hardcoded for the demo; registry lookup in production */
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        snprintf(out, out_size, "{\"error\":\"connect\"}");
        return;
    }

    char req[2048];
    int rlen = snprintf(req, sizeof(req),
        "POST %s HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        path, body ? strlen(body) : 0, body ? body : "");
    write(sock, req, rlen);

    char resp[4096];
    int total = 0;
    while (total < (int)sizeof(resp) - 1) {
        int n = read(sock, resp + total, sizeof(resp) - 1 - total);
        if (n <= 0) break;
        total += n;
    }
    resp[total] = '\0';
    close(sock);

    /* Extract body after \r\n\r\n */
    char *body = strstr(resp, "\r\n\r\n");
    if (body) {
        body += 4;
        snprintf(out, out_size, "%s", body);
    } else {
        snprintf(out, out_size, "%s", resp);
    }
}

static void handle_order_create(const HttpRequest *req, int client_fd) {
    char item[64] = "";
    int qty = 0;
    sscanf(req->body, "%*[^{]{\"item\":\"%63[^\"]\",\"quantity\":%d}", item, &qty);

    /* Step 1: check inventory */
    char check_req[256];
    snprintf(check_req, sizeof(check_req), "{\"item\":\"%s\",\"quantity\":%d}", item, qty);

    char check_resp[1024];
    post_to_inventory(ENDPOINT_INV_CHECK, check_req, check_resp, sizeof(check_resp));

    bool available = strstr(check_resp, "\"available\":true") != NULL;

    if (!available) {
        char inner[256];
        snprintf(inner, sizeof(inner),
                 "{\"item\":\"%s\",\"quantity\":%d,\"status\":\"rejected\",\"reason\":\"insufficient_stock\"}",
                 item, qty);
        char body[512];
        json_ok_response(body, sizeof(body), inner);
        http_send_json(client_fd, 200, body);
        return;
    }

    /* Step 2: reserve inventory */
    char order_id[32];
    snprintf(order_id, sizeof(order_id), "ORD-%d", (int)(unsigned long)req);

    char reserve_req[512];
    snprintf(reserve_req, sizeof(reserve_req),
             "{\"item\":\"%s\",\"quantity\":%d,\"order_id\":\"%s\"}", item, qty, order_id);

    char reserve_resp[1024];
    post_to_inventory(ENDPOINT_INV_RESERVE, reserve_req, reserve_resp, sizeof(reserve_resp));

    bool reserved = strstr(reserve_resp, "\"reserved\":true") != NULL;

    if (!reserved) {
        char inner[256];
        snprintf(inner, sizeof(inner),
                 "{\"order_id\":\"%s\",\"status\":\"rejected\",\"reason\":\"reserve_failed\"}", order_id);
        char body[512];
        json_ok_response(body, sizeof(body), inner);
        http_send_json(client_fd, 200, body);
        return;
    }

    /* Step 3: order accepted */
    char inner[256];
    snprintf(inner, sizeof(inner),
             "{\"order_id\":\"%s\",\"status\":\"accepted\",\"item\":\"%s\",\"quantity\":%d}",
             order_id, item, qty);
    char body[512];
    json_ok_response(body, sizeof(body), inner);
    http_send_json(client_fd, 200, body);
}

void order_handler(const HttpRequest *req, int client_fd) {
    if (strcmp(req->path, ENDPOINT_ORDER_CREATE) == 0 && strcmp(req->method, "POST") == 0) {
        handle_order_create(req, client_fd);
    } else {
        http_send_response(client_fd, 404, "Not Found", "text/plain", "not found");
    }
}
