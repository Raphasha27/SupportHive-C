#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <stdbool.h>

int db_init();
void db_close();

// Ticket structure
typedef struct {
    int id;
    char tenant_id[64];
    char title[256];
    char priority[16];
    char status[16];
    int escalation_level;
    char created_at[32];
} ticket_t;

// Escalation log
typedef struct {
    int id;
    int ticket_id;
    int level;
    char triggered_at[32];
} escalation_t;

// Multi-tenant ticket creation
int db_create_ticket(const char *tenant_id, const char *title, const char *priority, int *out_id);

// Multi-tenant ticket retrieval
int db_get_ticket(int id, const char *tenant_id, ticket_t *out_ticket);

// Ticket updates/Escalations
int db_update_escalation(int ticket_id, int level);
int db_log_escalation(int ticket_id, int level);

// Stats & Listing
int db_get_tickets(const char *tenant_id, ticket_t **out_tickets, int *count);
int db_get_stats(const char *tenant_id, int *open_count, int *breach_count);

#endif
