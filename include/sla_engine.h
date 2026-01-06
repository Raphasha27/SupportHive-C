#ifndef SLA_ENGINE_H
#define SLA_ENGINE_H

#include <uv.h>

typedef struct {
    int ticket_id;
    char tenant_id[64];
    uv_timer_t timer;
} sla_timer_t;

void sla_start_monitoring(uv_loop_t *loop, int ticket_id, const char *tenant_id, const char *priority);

#endif
