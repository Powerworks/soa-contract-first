#ifndef CIRCUIT_BREAKER_H
#define CIRCUIT_BREAKER_H

#include <stddef.h>
#include <time.h>

typedef enum {
    CB_CLOSED,    /* requests pass through */
    CB_OPEN,      /* requests fail fast */
    CB_HALF_OPEN  /* trial request allowed */
} CBState;

typedef struct {
    CBState   state;
    int       failure_count;
    int       failure_threshold;   /* trips after N failures */
    int       success_count;       /* successes needed to close from half-open */
    int       success_threshold;
    long      last_failure_ts;
    long      recovery_timeout_ms; /* time before trying half-open */
} CircuitBreaker;

void cb_init(CircuitBreaker *cb, int failure_threshold, int success_threshold, long recovery_timeout_ms);

/* Returns 1 if request is allowed, 0 if circuit is open (fail fast). */
int cb_allow_request(CircuitBreaker *cb);

/* Call after a successful call. */
void cb_on_success(CircuitBreaker *cb);

/* Call after a failed call. */
void cb_on_failure(CircuitBreaker *cb);

long now_ms(void);

#endif
