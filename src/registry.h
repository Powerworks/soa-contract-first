#ifndef REGISTRY_H
#define REGISTRY_H

#include "http_mini.h"

#define MAX_SERVICES 32
#define MAX_INSTANCES 8
#define HEALTHY_TTL_MS 10000

typedef struct {
    char address[64];
    long last_heartbeat_ms;
    int  healthy;
} ServiceInstance;

typedef struct {
    char name[64];
    ServiceInstance instances[MAX_INSTANCES];
    int instance_count;
} ServiceEntry;

void registry_handler(const HttpRequest *req, int client_fd);

/* Called by services to heartbeat their health */
void registry_heartbeat(const char *service_name, const char *address);

/* Get a healthy instance (round-robin) */
const ServiceInstance *registry_lookup_healthy(const char *name);

long now_ms(void);

#endif
