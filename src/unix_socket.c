#include "unix_socket.h"

bool open_unix_client(SocketState* sock_state, State* state) {
    socket_un server;

    if ((sock_state->server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("[UNIX_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, state->address);

    if (connect(sock_state->server_fd, (socket_t*)&server, sizeof(socket_un)) < 0) {
        printf("[UNIX_SOCKET] Couldn't connect to server.\n");
        return false;
    }
    
    return true;
}

bool open_unix_server(SocketState* sock_state, State* state) {
    socket_un server, client;

    if ((sock_state->server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("[UNIX_SOCKET] Couldn't open server socket.\n");
        return false;
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, state->address);
    unlink(server.sun_path);

    if (bind(sock_state->server_fd, (socket_t*)&server, sizeof(socket_un)) < 0) {
        printf("[UNIX_SOCKET] Couldn't create socket server.\n");
        return false;
    }

    if (listen(sock_state->server_fd, 5) < 0) {
        printf("[UNIX_SOCKET] Couldn't listen socket.\n");
        return false;
    }

    printf("[UNIX_SOCKET] Waiting client.\n");

    unsigned int len = sizeof(client); 
    if ((sock_state->client_fd = accept(sock_state->server_fd, (socket_t*)&client, &len)) < 0) {
        printf("[UNIX_SOCKET] Couldn't accept the client.\n");
        return false;
    }

    printf("[UNIX_SOCKET] Client connected.\n");
    
    return true;
}

void close_unix(SocketState* sock_state) {
    close(sock_state->server_fd);
    close(sock_state->client_fd);
}