#include "kimera/transport.h"

bool open_unix_client(SocketState* sock_state, State* state) {
    sock_state->server_un = (socket_un*)malloc(sizeof(socket_un));

    if ((sock_state->server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("[UNIX_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    sock_state->server_un->sun_family = AF_UNIX;
    strcpy(sock_state->server_un->sun_path, state->address);

    if (connect(sock_state->server_fd, (socket_t*)sock_state->server_un, sizeof(socket_un)) < 0) {
        printf("[UNIX_SOCKET] Couldn't connect to server.\n");
        return false;
    }
    
    sock_state->interf = UNIX;
    return true;
}

bool open_unix_server(SocketState* sock_state, State* state) {
    sock_state->server_un = (socket_un*)malloc(sizeof(socket_un));
    sock_state->client_un = (socket_un*)malloc(sizeof(socket_un));

    if ((sock_state->server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("[UNIX_SOCKET] Couldn't open server socket.\n");
        return false;
    }

    sock_state->server_un->sun_family = AF_UNIX;
    strcpy(sock_state->server_un->sun_path, state->address);
    unlink(sock_state->server_un->sun_path);

    if (bind(sock_state->server_fd, (socket_t*)sock_state->server_un, sizeof(socket_un)) < 0) {
        printf("[UNIX_SOCKET] Couldn't create socket server.\n");
        return false;
    }

    if (listen(sock_state->server_fd, 5) < 0) {
        printf("[UNIX_SOCKET] Couldn't listen socket.\n");
        return false;
    }

    printf("[UNIX_SOCKET] Waiting client.\n");

    unsigned int len = sizeof(socket_un); 
    if ((sock_state->client_fd = accept(sock_state->server_fd, (socket_t*)sock_state->client_un, &len)) < 0) {
        printf("[UNIX_SOCKET] Couldn't accept the client.\n");
        return false;
    }

    printf("[UNIX_SOCKET] Client connected.\n");
    
    sock_state->interf = UNIX;
    return true;
}

void close_unix(SocketState* sock_state) {
    close(sock_state->server_fd);
    close(sock_state->client_fd);
    sock_state->interf = NONE;
}