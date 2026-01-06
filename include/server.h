#ifndef SERVER_H
#define SERVER_H

#include <uv.h>
#include <http_parser.h>

#define DEFAULT_PORT 7000
#define BACKLOG 128

typedef struct {
    uv_tcp_t handle;
    http_parser parser;
    uv_write_t write_req;
} client_t;

// Initialize and start the HTTP server
int start_server(uv_loop_t *loop, int port);

#endif // SERVER_H
