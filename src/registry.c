#include "registry.h"
#include "json_mini.h"
#include <string.h>

#define MAX_SERVICES 32

typedef struct {
    char name[64];
    char address[64];
} ServiceEntry;

static ServiceEntry g_services[MAX_SERVICES];
static int g_service_count = 0;

static void handle_register(const HttpRequest *req, int client_fd) {
    char name[64] = "";
    char address[64] = "";
    sscanf(req->body, "%*[^{]{\"name\":\"%63[^\"]\",\"address\":\"%63[^\"]\"}", name, address);

    for (int i = 0; i < g_service_count; i++) {
        if (strcmp(g_services[i].name, name) == 0) {
            strcpy(g_services[i].address, address);
            char body[256];
            json_ok_response(body, sizeof(body), "{\"status\":\"updated\"}");
            http_send_json(client_fd, 200, body);
            return;
        }
    }

    if (g_service_count < MAX_SERVICES) {
        strcpy(g_services[g_service_count].name, name);
        strcpy(g_services[g_service_count].address, address);
        g_service_count++;
    }

    char body[256];
    json_ok_response(body, sizeof(body), "{\"status\":\"registered\"}");
    http_send_json(client_fd, 200, body);
}

static void handle_lookup(const HttpRequest *req, int client_fd) {
    char name[64] = "";
    sscanf(req->body, "%*[^{]{\"name\":\"%63[^\"]\"}", name);

    char address[64] = "not found";
    for (int i = 0; i < g_service_count; i++) {
        if (strcmp(g_services[i].name, name) == 0) {
            strcpy(address, g_services[i].address);
            break;
        }
    }

    char inner[128];
    snprintf(inner, sizeof(inner), "{\"name\":\"%s\",\"address\":\"%s\"}", name, address);
    char body[256];
    json_ok_response(body, sizeof(body), inner);
    http_send_json(client_fd, 200, body);
}

void registry_handler(const HttpRequest *req, int client_fd) {
    if (strcmp(req->path, "/register") == 0 && strcmp(req->method, "POST") == 0) {
        handle_register(req, client_fd);
    } else if (strcmp(req->path, "/lookup") == 0 && strcmp(req->method, "GET") == 0) {
        handle_lookup(req, client_fd);
    } else {
        http_send_response(client_fd, 404, "Not Found", "text/plain", "not found");
    }
}
