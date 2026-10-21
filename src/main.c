#include <stdio.h>
#include <string.h>

#include "http_mini.h"
#include "registry.h"
#include "inventory_service.h"
#include "order_service.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port> [role]\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    const char *role = (argc > 2) ? argv[2] : "service";

    if (strcmp(role, "registry") == 0) {
        http_server_run(port, registry_handler);
    } else if (strcmp(role, "inventory-service") == 0) {
        /* heartbeat to registry on startup */
        registry_heartbeat("inventory-service", "127.0.0.1:8082");
        http_server_run(port, inventory_handler);
    } else if (strcmp(role, "order-service") == 0) {
        registry_heartbeat("order-service", "127.0.0.1:8081");
        http_server_run(port, order_handler);
    } else {
        fprintf(stderr, "unknown role: %s\n", role);
        return 1;
    }

    return 0;
}
