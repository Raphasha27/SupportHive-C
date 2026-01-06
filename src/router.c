#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include "router.h"
#include "db.h"
#include "sla_engine.h"
#include "notifier.h"
#include "server.h"

extern void send_response(uv_stream_t *client, const char *status, const char *body, const char *content_type);

static char* read_dashboard_file() {
    FILE *f = fopen("dashboard/index.html", "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

void handle_stats(uv_stream_t *client, const char *tenant_id) {
    if (!tenant_id || strlen(tenant_id) == 0) {
        send_response(client, "400 Bad Request", "{\"error\":\"Missing X-Tenant-ID\"}", "application/json");
        return;
    }

    int open, breached;
    if (db_get_stats(tenant_id, &open, &breached) == 0) {
        char response[256];
        snprintf(response, sizeof(response), 
            "{\"tenant\":\"%s\", \"open_tickets\":%d, \"sla_breaches\":%d, \"status\":\"operational\"}", 
            tenant_id, open, breached);
        send_response(client, "200 OK", response, "application/json");
    } else {
        send_response(client, "500 Internal Error", "{\"error\":\"Stats failed\"}", "application/json");
    }
}

void handle_post_ticket(uv_stream_t *client, const char *tenant_id, const char *body) {
    if (!tenant_id || strlen(tenant_id) == 0) {
        send_response(client, "400 Bad Request", "{\"error\":\"Missing X-Tenant-ID\"}", "application/json");
        return;
    }

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        send_response(client, "400 Bad Request", "{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    cJSON *title = cJSON_GetObjectItemCaseSensitive(json, "title");
    cJSON *priority = cJSON_GetObjectItemCaseSensitive(json, "priority");

    if (!cJSON_IsString(title) || !cJSON_IsString(priority)) {
        send_response(client, "400 Bad Request", "{\"error\":\"Fields required\"}", "application/json");
        cJSON_Delete(json);
        return;
    }

    int ticket_id;
    if (db_create_ticket(tenant_id, title->valuestring, priority->valuestring, &ticket_id) == 0) {
        // Trigger Monitoring
        sla_start_monitoring(client->loop, ticket_id, tenant_id, priority->valuestring);
        
        // Notify
        notify_ticket_created(tenant_id, ticket_id);

        char response[128];
        snprintf(response, sizeof(response), "{\"status\":\"created\",\"id\":%d,\"sla\":\"active\"}", ticket_id);
        send_response(client, "201 Created", response, "application/json");
    } else {
        send_response(client, "500 Internal Error", "{\"error\":\"DB error\"}", "application/json");
    }

    cJSON_Delete(json);
}

void handle_list_tickets(uv_stream_t *client, const char *tenant_id) {
    if (!tenant_id || strlen(tenant_id) == 0) {
        send_response(client, "400 Bad Request", "{\"error\":\"Missing X-Tenant-ID\"}", "application/json");
        return;
    }

    ticket_t *tickets;
    int count;
    if (db_get_tickets(tenant_id, &tickets, &count) == 0) {
        cJSON *root = cJSON_CreateArray();
        for (int i = 0; i < count; i++) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", tickets[i].id);
            cJSON_AddStringToObject(item, "title", tickets[i].title);
            cJSON_AddStringToObject(item, "priority", tickets[i].priority);
            cJSON_AddStringToObject(item, "status", tickets[i].status);
            cJSON_AddNumberToObject(item, "escalation", tickets[i].escalation_level);
            cJSON_AddStringToObject(item, "created_at", tickets[i].created_at);
            cJSON_AddItemToArray(root, item);
        }
        char *json_out = cJSON_PrintUnformatted(root);
        send_response(client, "200 OK", json_out, "application/json");
        free(json_out);
        cJSON_Delete(root);
        if (tickets) free(tickets);
    } else {
        send_response(client, "500 Internal Error", "{\"error\":\"Fetch error\"}", "application/json");
    }
}

void handle_request(uv_stream_t *client, const char *method, const char *url, const char *tenant_id, const char *body) {
    if (strcmp(method, "OPTIONS") == 0) {
        send_response(client, "204 No Content", "", "text/plain");
        return;
    }

    if (strcmp(method, "POST") == 0 && strcmp(url, "/tickets") == 0) {
        handle_post_ticket(client, tenant_id, body);
    } else if (strcmp(method, "GET") == 0 && strcmp(url, "/tickets") == 0) {
        handle_list_tickets(client, tenant_id);
    } else if (strcmp(method, "GET") == 0 && strcmp(url, "/stats") == 0) {
        handle_stats(client, tenant_id);
    } else if (strcmp(method, "GET") == 0 && strcmp(url, "/dashboard") == 0) {
        char *html = read_dashboard_file();
        if (html) {
            send_response(client, "200 OK", html, "text/html");
            free(html);
        } else {
            send_response(client, "404 Not Found", "Dashboard file missing", "text/plain");
        }
    } else if (strcmp(url, "/") == 0) {
        send_response(client, "200 OK", "{\"service\":\"SupportHive-C\", \"version\":\"1.0.0\"}", "application/json");
    } else {
        send_response(client, "404 Not Found", "{\"error\":\"Not found\"}", "application/json");
    }
}
