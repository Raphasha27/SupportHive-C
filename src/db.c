#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"

static sqlite3 *db = NULL;

int db_init() {
    int rc = sqlite3_open("supporthive.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    const char *schema = 
        "CREATE TABLE IF NOT EXISTS tickets ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  tenant_id TEXT NOT NULL,"
        "  title TEXT NOT NULL,"
        "  priority TEXT NOT NULL,"
        "  status TEXT DEFAULT 'open',"
        "  escalation_level INTEGER DEFAULT 0,"
        "  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE TABLE IF NOT EXISTS escalations ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  ticket_id INTEGER NOT NULL,"
        "  level INTEGER NOT NULL,"
        "  triggered_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_tenant ON tickets(tenant_id);";

    char *err_msg = NULL;
    rc = sqlite3_exec(db, schema, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    printf("Database initialized successfully.\n");
    return 0;
}

void db_close() {
    if (db) sqlite3_close(db);
}

int db_create_ticket(const char *tenant_id, const char *title, const char *priority, int *out_id) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO tickets (tenant_id, title, priority) VALUES (?, ?, ?);";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 1;

    sqlite3_bind_text(stmt, 1, tenant_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, priority, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return 1;
    }

    if (out_id) *out_id = (int)sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return 0;
}

int db_get_ticket(int id, const char *tenant_id, ticket_t *out_ticket) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, tenant_id, title, priority, status, escalation_level, created_at FROM tickets WHERE id = ? AND tenant_id = ?;";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 1;

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_text(stmt, 2, tenant_id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out_ticket->id = sqlite3_column_int(stmt, 0);
        strncpy(out_ticket->tenant_id, (const char*)sqlite3_column_text(stmt, 1), 63);
        strncpy(out_ticket->title, (const char*)sqlite3_column_text(stmt, 2), 255);
        strncpy(out_ticket->priority, (const char*)sqlite3_column_text(stmt, 3), 15);
        strncpy(out_ticket->status, (const char*)sqlite3_column_text(stmt, 4), 15);
        out_ticket->escalation_level = sqlite3_column_int(stmt, 5);
        strncpy(out_ticket->created_at, (const char*)sqlite3_column_text(stmt, 6), 31);
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int db_update_escalation(int ticket_id, int level) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE tickets SET escalation_level = ? WHERE id = ?;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 1;
    sqlite3_bind_int(stmt, 1, level);
    sqlite3_bind_int(stmt, 2, ticket_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : 1;
}

int db_log_escalation(int ticket_id, int level) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO escalations (ticket_id, level) VALUES (?, ?);";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 1;
    sqlite3_bind_int(stmt, 1, ticket_id);
    sqlite3_bind_int(stmt, 2, level);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : 1;
}

int db_get_stats(const char *tenant_id, int *open_count, int *breach_count) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT "
                      "(SELECT COUNT(*) FROM tickets WHERE tenant_id = ? AND status = 'open'), "
                      "(SELECT COUNT(*) FROM tickets WHERE tenant_id = ? AND escalation_level > 0);";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 1;
    sqlite3_bind_text(stmt, 1, tenant_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, tenant_id, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *open_count = sqlite3_column_int(stmt, 0);
        *breach_count = sqlite3_column_int(stmt, 1);
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    return 1;
}
