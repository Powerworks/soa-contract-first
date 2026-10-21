#include "order_service.h"
#include "circuit_breaker.h"
#include <string.h>

static CircuitBreaker g_inv_cb;

static void post_to_inventory(const char *path, const char *body, char *out, size_t out_size) {
    (void)path; (void)body; (void)out; (void)out_size;
    snprintf(out, out_size, "{\"error\":\"circuit_breaker_placeholder\"}");
}

/* Order create with circuit breaker around inventory calls */
static void handle_order_create(const HttpRequest *req, int client_fd) {
    char item[64] = "";
    int qty = 0;
    sscanf(req->body, "%*[^{]{\"item\":\"%63[^\"]\",\"quantity\":%d}", item, &qty);

    if (!cb_allow_request(&g_inv_cb)) {
        char inner[256];
        snprintf(inner, sizeof(inner),
                 "{\"item\":\"%s\",\"quantity\":%d,\"status\":\"rejected\",\"reason\":\"circuit_open\"}",
                 item, qty);
        char body[512];
        json_ok_response(body, sizeof(body), inner);
        http_send_json(client_fd, 200, body);
        return;
    }

    char check_req[256];
    snprintf(check_req, sizeof(check_req), "{\"item\":\"%s\",\"quantity\":%d}", item, qty);

    char check_resp[1024];
    post_to_inventory(ENDPOINT_INV_CHECK, check_req, check_resp, sizeof(check_resp));

    int available = strstr(check_resp, "\"available\":true") != NULL;

    if (!available) {
        cb_on_failure(&g_inv_cb);
        char inner[256];
        snprintf(inner, sizeof(inner),
                 "{\"item\":\"%s\",\"quantity\":%d,\"status\":\"rejected\",\"reason\":\"insufficient_stock\"}",
                 item, qty);
        char body[512];
        json_ok_response(body, sizeof(body), inner);
        http_send_json(client_fd, 200, body);
        return;
    }

    cb_on_success(&g_inv_cb);

    char order_id[32];
    snprintf(order_id, sizeof(order_id), "ORD-%d", (int)(unsigned long)req);

    char reserve_req[512];
    snprintf(reserve_req, sizeof(reserve_req),
             "{\"item\":\"%s\",\"quantity\":%d,\"order_id\":\"%s\"}", item, qty, order_id);

    char reserve_resp[1024];
    post_to_inventory(ENDPOINT_INV_RESERVE, reserve_req, reserve_resp, sizeof(reserve_resp));

    int reserved = strstr(reserve_resp, "\"reserved\":true") != NULL;

    if (!reserved) {
        cb_on_failure(&g_inv_cb);
        char inner[256];
        snprintf(inner, sizeof(inner),
                 "{\"order_id\":\"%s\",\"status\":\"rejected\",\"reason\":\"reserve_failed\"}", order_id);
        char body[512];
        json_ok_response(body, sizeof(body), inner);
        http_send_json(client_fd, 200, body);
        return;
    }

    cb_on_success(&g_inv_cb);

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
