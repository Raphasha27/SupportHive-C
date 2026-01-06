#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sla_engine.h"
#include "db.h"
#include "notifier.h"

static void on_sla_breach(uv_timer_t *handle) {
    sla_timer_t *context = (sla_timer_t*)handle->data;
    
    printf("\n🚨 [SLA BREACH ALERT] Ticket %d (Tenant: %s) triggered!\n", context->ticket_id, context->tenant_id);

    // Persist breach in DB
    db_update_escalation(context->ticket_id, 1);
    db_log_escalation(context->ticket_id, 1);

    // Send notification
    notify_escalation(context->tenant_id, context->ticket_id, 1);

    // Stop and cleanup
    uv_timer_stop(handle);
    free(context);
}

void sla_start_monitoring(uv_loop_t *loop, int ticket_id, const char *tenant_id, const char *priority) {
    sla_timer_t *context = (sla_timer_t*)malloc(sizeof(sla_timer_t));
    context->ticket_id = ticket_id;
    strncpy(context->tenant_id, tenant_id, 63);

    uv_timer_init(loop, &context->timer);
    context->timer.data = context;

    uint64_t timeout_ms = 60000;

    if (strcmp(priority, "P1") == 0) timeout_ms = 5000;      // 5s for fast P1 demo
    else if (strcmp(priority, "P2") == 0) timeout_ms = 15000; // 15s for P2 demo

    printf("⏱️ SLA monitor active for #%d (Breach in %llu ms)\n", ticket_id, timeout_ms);

    uv_timer_start(&context->timer, on_sla_breach, timeout_ms, 0);
}
