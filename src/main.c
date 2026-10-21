#include <stdio.h>
#include <string.h>

#include "http_mini.h"
#include "registry.h"
#include "inventory_service.h"
#include "order_service.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port> [registry_addr]\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    const char *role = strrchr(argv[0], '/');
    role = role ? role + 1 : argv[0];

    if (strcmp(role, "registry") == 0) {
        http_server_run(port, registry_handler);
    } else if (strcmp(role, "inventory-service") == 0) {
        http_server_run(port, inventory_handler);
    } else if (strcmp(role, "order-service") == 0) {
        http_server_run(port, order_handler);
    } else {
        fprintf(stderr, "unknown role: %s\n", role);
        return 1;
    }

    return 0;
}
