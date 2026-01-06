#ifndef ROUTER_H
#define ROUTER_H

#include <uv.h>
#include <http_parser.h>

void handle_request(uv_stream_t *client, const char *method, const char *url, const char *tenant_id, const char *body);

#endif
