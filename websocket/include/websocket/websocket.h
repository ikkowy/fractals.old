#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stddef.h>

typedef struct ws_connection ws_connection;

typedef void (*ws_open_callback)(
    ws_connection* connection,
    void* userdata
);

typedef void (*ws_close_callback)(
    ws_connection* connection,
    void* data,
    size_t size,
    void* userdata
);

typedef void (*ws_message_callback)(
    ws_connection* connection,
    void* userdata
);

void ws_run_server(
    int port,
    ws_open_callback open_callback,
    ws_close_callback close_callback,
    ws_message_callback message_callback,
    void* userdata
);

void ws_send(
    ws_connection* connection,
    void* data,
    size_t size
);

#endif /* WEBSOCKET_H */
