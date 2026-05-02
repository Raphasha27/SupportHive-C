/**
 * EventHive-C | Advanced Analytics Engine
 * ============================================================
 * High-performance real-time data aggregation for event metrics.
 * 
 * Features:
 * - Circular buffer for time-series data
 * - Weighted moving averages for task latency
 * - Tenant-level metric bucketing
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "include/analytics.h"

#define MAX_TENANTS 100
#define WINDOW_SIZE 60 // 60-second window

typedef struct {
    char tenant_id[64];
    uint32_t task_counts[WINDOW_SIZE];
    uint32_t escalation_counts[WINDOW_SIZE];
    double avg_latency[WINDOW_SIZE];
    uint32_t current_index;
    time_t last_update;
} TenantMetrics;

static TenantMetrics* metrics_store[MAX_TENANTS];
static int tenant_count = 0;

/**
 * Finds or creates a metric bucket for a tenant
 */
static TenantMetrics* get_tenant_metrics(const char* tenant_id) {
    for (int i = 0; i < tenant_count; i++) {
        if (strcmp(metrics_store[i]->tenant_id, tenant_id) == 0) {
            return metrics_store[i];
        }
    }
    
    if (tenant_count >= MAX_TENANTS) return NULL;
    
    TenantMetrics* m = (TenantMetrics*)calloc(1, sizeof(TenantMetrics));
    strncpy(m->tenant_id, tenant_id, 63);
    m->last_update = time(NULL);
    metrics_store[tenant_count++] = m;
    return m;
}

/**
 * Updates the circular buffer index based on time
 */
static void sync_time_window(TenantMetrics* m) {
    time_t now = time(NULL);
    uint32_t diff = (uint32_t)(now - m->last_update);
    
    if (diff > 0) {
        if (diff >= WINDOW_SIZE) {
            memset(m->task_counts, 0, sizeof(m->task_counts));
            memset(m->escalation_counts, 0, sizeof(m->escalation_counts));
            memset(m->avg_latency, 0, sizeof(m->avg_latency));
            m->current_index = 0;
        } else {
            for (uint32_t i = 1; i <= diff; i++) {
                uint32_t idx = (m->current_index + i) % WINDOW_SIZE;
                m->task_counts[idx] = 0;
                m->escalation_counts[idx] = 0;
                m->avg_latency[idx] = 0;
            }
            m->current_index = (m->current_index + diff) % WINDOW_SIZE;
        }
        m->last_update = now;
    }
}

/**
 * Records a task event in the analytics engine
 */
void analytics_record_task(const char* tenant_id, int priority, double latency) {
    TenantMetrics* m = get_tenant_metrics(tenant_id);
    if (!m) return;
    
    sync_time_window(m);
    
    m->task_counts[m->current_index]++;
    
    // Weighted moving average for latency
    double old_avg = m->avg_latency[m->current_index];
    uint32_t count = m->task_counts[m->current_index];
    m->avg_latency[m->current_index] = old_avg + (latency - old_avg) / count;
}

/**
 * Records an escalation event
 */
void analytics_record_escalation(const char* tenant_id) {
    TenantMetrics* m = get_tenant_metrics(tenant_id);
    if (!m) return;
    
    sync_time_window(m);
    m->escalation_counts[m->current_index]++;
}

/**
 * Aggregates metrics for a given duration (seconds)
 */
void analytics_get_summary(const char* tenant_id, int duration, uint32_t* total_tasks, uint32_t* total_escalations) {
    TenantMetrics* m = get_tenant_metrics(tenant_id);
    if (!m) {
        *total_tasks = 0;
        *total_escalations = 0;
        return;
    }
    
    sync_time_window(m);
    
    uint32_t tasks = 0;
    uint32_t escalations = 0;
    int limit = (duration > WINDOW_SIZE) ? WINDOW_SIZE : duration;
    
    for (int i = 0; i < limit; i++) {
        int idx = (m->current_index - i + WINDOW_SIZE) % WINDOW_SIZE;
        tasks += m->task_counts[idx];
        escalations += m->escalation_counts[idx];
    }
    
    *total_tasks = tasks;
    *total_escalations = escalations;
}

/**
 * Internal cleanup
 */
void analytics_shutdown() {
    for (int i = 0; i < tenant_count; i++) {
        free(metrics_store[i]);
    }
    tenant_count = 0;
}

/**
 * Diagnostic dump of analytics data
 */
void analytics_dump_debug() {
    printf("\n--- Analytics Engine Dump ---\n");
    for (int i = 0; i < tenant_count; i++) {
        TenantMetrics* m = metrics_store[i];
        uint32_t t, e;
        analytics_get_summary(m->tenant_id, 60, &t, &e);
        printf("Tenant: %s | Tasks (60s): %u | Escalations: %u\n", m->tenant_id, t, e);
    }
    printf("---------------------------\n");
}
