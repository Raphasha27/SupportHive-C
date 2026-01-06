#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include "router.h"
#include "db.h"
#include "sla_engine.h"
#include "notifier.h"
#include "server.h"

extern void send_response(uv_stream_t *client, const char *status, const char *body);

void handle_stats(uv_stream_t *client, const char *tenant_id) {
    if (!tenant_id || strlen(tenant_id) == 0) {
        send_response(client, "400 Bad Request", "{\"error\":\"Missing X-Tenant-ID\"}");
        return;
    }

    int open, breached;
    if (db_get_stats(tenant_id, &open, &breached) == 0) {
        char response[256];
        snprintf(response, sizeof(response), 
            "{\"tenant\":\"%s\", \"open_tickets\":%d, \"sla_breaches\":%d, \"status\":\"operational\"}", 
            tenant_id, open, breached);
        send_response(client, "200 OK", response);
    } else {
        send_response(client, "500 Internal Error", "{\"error\":\"Stats failed\"}");
    }
}

void handle_post_ticket(uv_stream_t *client, const char *tenant_id, const char *body) {
    if (!tenant_id || strlen(tenant_id) == 0) {
        send_response(client, "400 Bad Request", "{\"error\":\"Missing X-Tenant-ID\"}");
        return;
    }

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        send_response(client, "400 Bad Request", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    cJSON *title = cJSON_GetObjectItemCaseSensitive(json, "title");
    cJSON *priority = cJSON_GetObjectItemCaseSensitive(json, "priority");

    if (!cJSON_IsString(title) || !cJSON_IsString(priority)) {
        send_response(client, "400 Bad Request", "{\"error\":\"Fields required\"}");
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
        send_response(client, "201 Created", response);
    } else {
        send_response(client, "500 Internal Error", "{\"error\":\"DB error\"}");
    }

    cJSON_Delete(json);
}

void handle_request(uv_stream_t *client, const char *method, const char *url, const char *tenant_id, const char *body) {
    if (strcmp(method, "POST") == 0 && strcmp(url, "/tickets") == 0) {
        handle_post_ticket(client, tenant_id, body);
    } else if (strcmp(method, "GET") == 0 && strcmp(url, "/stats") == 0) {
        handle_stats(client, tenant_id);
    } else if (strcmp(url, "/") == 0) {
        send_response(client, "200 OK", "{\"service\":\"SupportHive-C\", \"version\":\"1.0.0\"}");
    } else {
        send_response(client, "404 Not Found", "{\"error\":\"Not found\"}");
    }
}
