#include "registry.h"
#include "json_mini.h"
#include <string.h>

static ServiceEntry g_services[MAX_SERVICES];
static int g_service_count = 0;
static int g_rr_index[MAX_SERVICES]; /* round-robin per service */

static long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static ServiceEntry *find_service(const char *name) {
    for (int i = 0; i < g_service_count; i++)
        if (strcmp(g_services[i].name, name) == 0) return &g_services[i];
    return NULL;
}

void registry_heartbeat(const char *service_name, const char *address) {
    ServiceEntry *svc = find_service(service_name);
    if (!svc && g_service_count < MAX_SERVICES) {
        svc = &g_services[g_service_count++];
        strcpy(svc->name, service_name);
        svc->instance_count = 0;
    }
    if (!svc) return;

    /* Find or create instance */
    ServiceInstance *inst = NULL;
    for (int i = 0; i < svc->instance_count; i++)
        if (strcmp(svc->instances[i].address, address) == 0) { inst = &svc->instances[i]; break; }
    if (!inst && svc->instance_count < MAX_INSTANCES) {
        inst = &svc->instances[svc->instance_count++];
        strcpy(inst->address, address);
        inst->healthy = 1;
        inst->last_heartbeat_ms = 0;
    }
    if (inst) {
        inst->last_heartbeat_ms = now_ms();
        inst->healthy = 1;
    }
}

const ServiceInstance *registry_lookup_healthy(const char *name) {
    ServiceEntry *svc = find_service(name);
    if (!svc || svc->instance_count == 0) return NULL;

    /* Mark stale instances unhealthy */
    long now = now_ms();
    int healthy_count = 0;
    for (int i = 0; i < svc->instance_count; i++) {
        if (now - svc->instances[i].last_heartbeat_ms > HEALTHY_TTL_MS)
            svc->instances[i].healthy = 0;
        if (svc->instances[i].healthy) healthy_count++;
    }
    if (healthy_count == 0) return NULL;

    /* Round-robin among healthy instances */
    int start = g_rr_index[name[0] % MAX_SERVICES] % svc->instance_count;
    for (int i = 0; i < svc->instance_count; i++) {
        int idx = (start + i) % svc->instance_count;
        if (svc->instances[idx].healthy) {
            g_rr_index[name[0] % MAX_SERVICES] = (idx + 1) % svc->instance_count;
            return &svc->instances[idx];
        }
    }
    return NULL;
}

static void handle_register(const HttpRequest *req, int client_fd) {
    char name[64] = "";
    char address[64] = "";
    sscanf(req->body, "%*[^{]{\"name\":\"%63[^\"]\",\"address\":\"%63[^\"]\"}", name, address);
    registry_heartbeat(name, address);
    char body[256];
    json_ok_response(body, sizeof(body), "{\"status\":\"registered\"}");
    http_send_json(client_fd, 200, body);
}

static void handle_heartbeat(const HttpRequest *req, int client_fd) {
    char name[64] = "";
    char address[64] = "";
    sscanf(req->body, "%*[^{]{\"name\":\"%63[^\"]\",\"address\":\"%63[^\"]\"}", name, address);
    registry_heartbeat(name, address);
    char body[256];
    json_ok_response(body, sizeof(body), "{\"status\":\"ok\"}");
    http_send_json(client_fd, 200, body);
}

static void handle_lookup(const HttpRequest *req, int client_fd) {
    char name[64] = "";
    sscanf(req->body, "%*[^{]{\"name\":\"%63[^\"]\"}", name);

    const ServiceInstance *inst = registry_lookup_healthy(name);
    char body[256];
    if (inst) {
        char inner[128];
        snprintf(inner, sizeof(inner), "{\"name\":\"%s\",\"address\":\"%s\",\"healthy\":true}", name, inst->address);
        json_ok_response(body, sizeof(body), inner);
    } else {
        char inner[64];
        snprintf(inner, sizeof(inner), "{\"name\":\"%s\",\"error\":\"no_healthy_instances\"}", name);
        snprintf(body, sizeof(body), "{\"ok\":false,\"error\":%s}", inner);
    }
    http_send_json(client_fd, inst ? 200 : 503, body);
}

static void handle_health(const HttpRequest *req, int client_fd) {
    /* Registry self-health */
    char body[128];
    json_ok_response(body, sizeof(body), "{\"status\":\"healthy\",\"service_count\":%d}");
    http_send_json(client_fd, 200, body);
}

void registry_handler(const HttpRequest *req, int client_fd) {
    if (strcmp(req->path, "/register") == 0 && strcmp(req->method, "POST") == 0) {
        handle_register(req, client_fd);
    } else if (strcmp(req->path, "/heartbeat") == 0 && strcmp(req->method, "POST") == 0) {
        handle_heartbeat(req, client_fd);
    } else if (strcmp(req->path, "/lookup") == 0 && strcmp(req->method, "GET") == 0) {
        handle_lookup(req, client_fd);
    } else if (strcmp(req->path, "/health") == 0 && strcmp(req->method, "GET") == 0) {
        handle_health(req, client_fd);
    } else {
        http_send_response(client_fd, 404, "Not Found", "text/plain", "not found");
    }
}
