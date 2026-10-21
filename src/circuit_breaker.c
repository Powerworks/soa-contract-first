#include "circuit_breaker.h"
#include <stdio.h>

long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

void cb_init(CircuitBreaker *cb, int failure_threshold, int success_threshold, long recovery_timeout_ms) {
    cb->state = CB_CLOSED;
    cb->failure_count = 0;
    cb->failure_threshold = failure_threshold;
    cb->success_count = 0;
    cb->success_threshold = success_threshold;
    cb->last_failure_ts = 0;
    cb->recovery_timeout_ms = recovery_timeout_ms;
}

int cb_allow_request(CircuitBreaker *cb) {
    long now = now_ms();

    switch (cb->state) {
        case CB_CLOSED:
            return 1;
        case CB_OPEN:
            if (now - cb->last_failure_ts >= cb->recovery_timeout_ms) {
                cb->state = CB_HALF_OPEN;
                cb->success_count = 0;
                return 1; /* trial request */
            }
            return 0; /* fail fast */
        case CB_HALF_OPEN:
            return 1; /* trial request */
        default:
            return 0;
    }
}

void cb_on_success(CircuitBreaker *cb) {
    switch (cb->state) {
        case CB_CLOSED:
            cb->failure_count = 0;
            break;
        case CB_HALF_OPEN:
            cb->success_count++;
            if (cb->success_count >= cb->success_threshold) {
                cb->state = CB_CLOSED;
                cb->failure_count = 0;
                cb->success_count = 0;
            }
            break;
        case CB_OPEN:
            break;
    }
}

void cb_on_failure(CircuitBreaker *cb) {
    cb->failure_count++;
    cb->last_failure_ts = now_ms();

    if (cb->state == CB_HALF_OPEN) {
        cb->state = CB_OPEN;
        cb->success_count = 0;
    } else if (cb->state == CB_CLOSED && cb->failure_count >= cb->failure_threshold) {
        cb->state = CB_OPEN;
    }
}
