#include <stdio.h>
#include <stdlib.h>

#include <websocket/websocket.h>

static void onopen(
    ws_connection* connection,
    void* userdata
) {
    puts("open");
}

static void onclose(
    ws_connection* connection,
    void* data,
    size_t size,
    void* userdata
) {
    puts("close");
}

static void onmessage(
    ws_connection* connection,
    void* userdata
) {
    puts("message");
}

int main() {
    ws_run_server(8080, onopen, onclose, onmessage, NULL);
    return EXIT_SUCCESS;
}
