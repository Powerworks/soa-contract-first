#include "inventory_service.h"
#include "contracts.h"
#include "json_mini.h"
#include <string.h>

static int g_stock_widget = 100;
static int g_stock_gadget = 50;

static int stock_for(const char *item) {
    if (strcmp(item, "widget") == 0) return g_stock_widget;
    if (strcmp(item, "gadget") == 0) return g_stock_gadget;
    return 0;
}

static void set_stock(const char *item, int v) {
    if (strcmp(item, "widget") == 0) g_stock_widget = v;
    if (strcmp(item, "gadget") == 0) g_stock_gadget = v;
}

static void handle_check(const HttpRequest *req, int client_fd) {
    char item[64] = "";
    int qty = 0;
    sscanf(req->body, "%*[^{]{\"item\":\"%63[^\"]\",\"quantity\":%d}", item, &qty);

    int available = stock_for(item) >= qty;
    char inner[128];
    snprintf(inner, sizeof(inner), "{\"item\":\"%s\",\"available\":%s}", item,
             available ? "true" : "false");
    char body[256];
    json_ok_response(body, sizeof(body), inner);
    http_send_json(client_fd, 200, body);
}

static void handle_reserve(const HttpRequest *req, int client_fd) {
    char item[64] = "";
    char order_id[64] = "";
    int qty = 0;
    sscanf(req->body, "%*[^{]{\"item\":\"%63[^\"]\",\"quantity\":%d,\"order_id\":\"%63[^\"]\"}",
           item, &qty, order_id);

    bool reserved = false;
    if (stock_for(item) >= qty) {
        set_stock(item, stock_for(item) - qty);
        reserved = true;
    }

    char inner[256];
    snprintf(inner, sizeof(inner),
             "{\"item\":\"%s\",\"quantity\":%d,\"order_id\":\"%s\",\"reserved\":%s}",
             item, qty, order_id, reserved ? "true" : "false");
    char body[256];
    json_ok_response(body, sizeof(body), inner);
    http_send_json(client_fd, 200, body);
}

void inventory_handler(const HttpRequest *req, int client_fd) {
    if (strcmp(req->path, ENDPOINT_INV_CHECK) == 0 && strcmp(req->method, "POST") == 0) {
        handle_check(req, client_fd);
    } else if (strcmp(req->path, ENDPOINT_INV_RESERVE) == 0 && strcmp(req->method, "POST") == 0) {
        handle_reserve(req, client_fd);
    } else {
        http_send_response(client_fd, 404, "Not Found", "text/plain", "not found");
    }
}
