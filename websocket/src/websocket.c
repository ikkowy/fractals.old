#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <utils/base64.h>
#include <utils/sha1.h>
#include <utils/strops.h>

#include <websocket/websocket.h>

#define MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

enum {
    WS_HANDSHAKE_SUCCESS,
    WS_HANDSHAKE_FAILURE
};

static void gen_token(const char* key, char* token) {
    char sec[60];
    char hash[SHA1_HASH_SIZE];
    memcpy(sec, key, 24);
    memcpy(sec + 24, MAGIC_STRING, 36);
    sha1((uint8_t*) sec, 60, (uint8_t*) hash);
    base64_encode((uint8_t*) hash, SHA1_HASH_SIZE, token);
}

int ws_handshake_with_client(
    int client_socket
) {
    const int BUFFER_SIZE = 4096;
    const int MAX_LINE_SIZE = 1024;
    ssize_t size;
    char buffer[BUFFER_SIZE];
    char line_buffer[MAX_LINE_SIZE + 1];
    char token[29];
    char response[1024];
    const char* line_;
    char c;
    int i, j = 0;
    bool eol = false;

    /* Receive request. */

    size = read(client_socket, buffer, BUFFER_SIZE);

    if (size == -1) {
        perror("websocket: read");
        return WS_HANDSHAKE_FAILURE;
    }

    /* Parse request. */

    for (i = 0; i < size; i++) {
        c = buffer[i];
        if (c == '\r');
        else if (c == '\n') {
            if (eol) break;


            line_buffer[j] = '\0';

            line_ = line_buffer;

            if (starts_with_ci(line_, "Sec-WebSocket-Key:")) {
                line_ = skip_str_ci(line_, "Sec-WebSocket-Key:");
                line_ = skip_lws(line_);
                if (strlen(line_) != 24) return WS_HANDSHAKE_FAILURE;
                gen_token(line_, token);
            }

            j = 0;
            eol = true;
        } else {
            if (j == MAX_LINE_SIZE) return WS_HANDSHAKE_FAILURE;
            line_buffer[j++] = c;
            eol = false;
        }
    }

    /* Send handshake response to client. */

    size = sprintf(
        response,
        "HTTP/1.1 101 Switching Protocols\r\n" \
        "Upgrade: websocket\r\n" \
        "Connection: Upgrade\r\n" \
        "Sec-WebSocket-Accept: %s\r\n" \
        "\r\n",
        token
    );

    write(client_socket, response, size);

    return WS_HANDSHAKE_SUCCESS;
}

/* -------------------------------------------------------------------------- */

void ws_run_server(
    int port,
    ws_open_callback open_callback,
    ws_close_callback close_callback,
    ws_message_callback message_callback,
    void* userdata
) {
    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addrlen;
    int status;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        perror("websocket: socket");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    status = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));

    if (status == -1) {
        perror("websocket: bind");
        exit(EXIT_FAILURE);
    }

    status = listen(server_socket, 20);

    if (status == -1) {
        perror("websocket: listen");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        addrlen = sizeof(client_addr);

        client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &addrlen);

        if (client_socket == -1) {
            perror("websocket: accept");
            continue;
        }

        fprintf(stderr, "websocket: client connected: %s (%d)\n", inet_ntoa(client_addr.sin_addr), client_socket);

        status = ws_handshake_with_client(client_socket);

        if (status == WS_HANDSHAKE_SUCCESS) {
            fprintf(stderr, "websocket: handshake with client (%d): success\n", client_socket);
        } else {
            fprintf(stderr, "websocket: handshake with client (%d): failure\n", client_socket);
        }

        close(client_socket);
    }

    close(server_socket);
}
