#include <stdio.h>
#include <uv.h>
#include "server.h"
#include "db.h"

int main() {
    printf("Starting SupportHive-C Engine...\n");
    printf("Framework: libuv + http-parser + sqlite3 + cjson\n");

    // Initialize Database
    if (db_init() != 0) {
        return 1;
    }

    uv_loop_t *loop = uv_default_loop();

    if (start_server(loop, DEFAULT_PORT) != 0) {
        fprintf(stderr, "Failed to start server.\n");
        return 1;
    }

    // Run the event loop
    uv_run(loop, UV_RUN_DEFAULT);

    // Cleanup
    db_close();
    
    return 0;
}
