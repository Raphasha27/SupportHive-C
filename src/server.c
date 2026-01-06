#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "router.h"

// Client-specific state for http-parser
typedef struct {
    uv_tcp_t handle;
    http_parser parser;
    char last_header_field[64];
    char tenant_id[64];
    char body[2048];
    char url[256];
    char method[16];
} http_client_t;

// callbacks for http-parser
int on_url(http_parser* p, const char* at, size_t length) {
    http_client_t* client = (http_client_t*)p->data;
    strncat(client->url, at, length);
    return 0;
}

int on_header_field(http_parser* p, const char* at, size_t length) {
    http_client_t* client = (http_client_t*)p->data;
    strncpy(client->last_header_field, at, length);
    client->last_header_field[length] = '\0';
    return 0;
}

int on_header_value(http_parser* p, const char* at, size_t length) {
    http_client_t* client = (http_client_t*)p->data;
    if (strcmp(client->last_header_field, "X-Tenant-ID") == 0) {
        strncpy(client->tenant_id, at, length);
        client->tenant_id[length] = '\0';
    }
    return 0;
}

int on_body(http_parser* p, const char* at, size_t length) {
    http_client_t* client = (http_client_t*)p->data;
    strncat(client->body, at, length);
    return 0;
}

int on_message_complete(http_parser* p) {
    http_client_t* client = (http_client_t*)p->data;
    strncpy(client->method, http_method_str(p->method), 15);
    
    handle_request((uv_stream_t*)&client->handle, client->method, client->url, client->tenant_id, client->body);
    return 0;
}

static http_parser_settings settings = {
    .on_url = on_url,
    .on_header_field = on_header_field,
    .on_header_value = on_header_value,
    .on_body = on_body,
    .on_message_complete = on_message_complete
};

void on_new_connection(uv_stream_t *server, int status);
void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
void on_close(uv_handle_t *handle);

void send_response(uv_stream_t *client, const char *status, const char *body) {
    char response[1024];
    snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status, strlen(body), body);

    uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_buf_t buf = uv_buf_init(strdup(response), strlen(response));
    
    uv_write(req, client, &buf, 1, NULL); 
}

int start_server(uv_loop_t *loop, int port) {
    uv_tcp_t *server = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    struct sockaddr_in addr;

    uv_tcp_init(loop, server);
    uv_ip4_addr("0.0.0.0", port, &addr);

    uv_tcp_bind(server, (const struct sockaddr*)&addr, 0);
    uv_listen((uv_stream_t*)server, BACKLOG, on_new_connection);

    printf("🐝 SupportHive-C engine listening on port %d\n", port);
    return 0;
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) return;

    http_client_t *client = (http_client_t*)malloc(sizeof(http_client_t));
    memset(client, 0, sizeof(http_client_t));
    
    uv_tcp_init(server->loop, &client->handle);
    client->handle.data = client;
    client->parser.data = client;
    http_parser_init(&client->parser, HTTP_REQUEST);

    if (uv_accept(server, (uv_stream_t*)&client->handle) == 0) {
        uv_read_start((uv_stream_t*)&client->handle, on_alloc, on_read);
    } else {
        uv_close((uv_handle_t*)&client->handle, on_close);
    }
}

void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    http_client_t *client = (http_client_t*)stream->data;
    if (nread > 0) {
        size_t parsed = http_parser_execute(&client->parser, &settings, buf->base, nread);
        if (parsed < nread) {
            uv_close((uv_handle_t*)stream, on_close);
        }
    } else {
        uv_close((uv_handle_t*)stream, on_close);
    }
    if (buf->base) free(buf->base);
}

void on_close(uv_handle_t *handle) {
    free(handle);
}
