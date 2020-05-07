#include "tcp_socket.h"

bool open_tcp_client(SocketState* sock_state, State* state) {
    socket_in server;

    if ((sock_state->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(state->port);

    if (inet_pton(AF_INET, state->address, &server.sin_addr) < 0) { 
        printf("[TCP_SOCKET] Invalid address.\n"); 
        return false;
    } 

    if (connect(sock_state->server_fd, (socket_t*)&server, sizeof(socket_in)) < 0) {
        printf("[TCP_SOCKET] Couldn't connect to server.\n");
        return false;
    }

    return true;
}

bool open_tcp_server(SocketState* sock_state, State* state) {
    socket_in server, client;

    if ((sock_state->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }
    bzero(&server, sizeof(server)); 
  
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(state->port); 

    if (bind(sock_state->server_fd, (socket_t*)&server, sizeof(socket_in)) < 0) {
        printf("[TCP_SOCKET] Couldn't open server port.\n");
        return false;
    }

    if ((listen(sock_state->server_fd, 5)) != 0) { 
        printf("[TCP_SOCKET] Failed to listen.\n");
        return false;
    }

    printf("[TCP_SOCKET] Waiting client.\n");

    unsigned int len = sizeof(client); 
    if ((sock_state->client_fd = accept(sock_state->server_fd, (socket_t*)&client, &len)) < 0) {
        printf("[TCP_SOCKET] Couldn't accept the client.\n");
        return false;
    }

    printf("[TCP_SOCKET] Client connected.\n");

    return true;
}

void close_tcp(SocketState* sock_state) {
    close(sock_state->client_fd);
    close(sock_state->server_fd);
}