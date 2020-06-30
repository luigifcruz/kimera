#include "kimera/transport.h"

bool open_tcp_client(SocketState* sock_state, State* state) {
    sock_state->server_in = (socket_in*)malloc(sizeof(socket_in));

    if ((sock_state->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    memset(sock_state->server_in, 0, sizeof(socket_in)); 

    sock_state->server_in->sin_family = AF_INET;
    sock_state->server_in->sin_port = htons(state->port);

    if (inet_pton(AF_INET, state->address, &sock_state->server_in->sin_addr) < 0) { 
        printf("[TCP_SOCKET] Invalid address.\n"); 
        return false;
    } 

    if (connect(sock_state->server_fd, (socket_t*)sock_state->server_in, sizeof(socket_in)) < 0) {
        printf("[TCP_SOCKET] Couldn't connect to server.\n");
        return false;
    }

    sock_state->interf = TCP;
    return true;
}

bool open_tcp_server(SocketState* sock_state, State* state) {
    sock_state->server_in = (socket_in*)malloc(sizeof(socket_in));
    sock_state->client_in = (socket_in*)malloc(sizeof(socket_in));

    if ((sock_state->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    if (setsockopt(sock_state->server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        printf("[TCP_SOCKET] Failed trying to reuse socket.\n");
    }
    
    memset(sock_state->server_in, 0, sizeof(socket_in)); 
    memset(sock_state->client_in, 0, sizeof(socket_in)); 

    sock_state->server_in->sin_family = AF_INET; 
    sock_state->server_in->sin_addr.s_addr = htonl(INADDR_ANY); 
    sock_state->server_in->sin_port = htons(state->port); 

    if (bind(sock_state->server_fd, (socket_t*)sock_state->server_in, sizeof(socket_in)) < 0) {
        printf("[TCP_SOCKET] Couldn't open server port.\n");
        return false;
    }

    if ((listen(sock_state->server_fd, 1)) != 0) { 
        printf("[TCP_SOCKET] Failed to listen.\n");
        return false;
    }

    printf("[TCP_SOCKET] Waiting client.\n");

    unsigned int len = sizeof(socket_in); 
    if ((sock_state->client_fd = accept(sock_state->server_fd, (socket_t*)sock_state->client_in, &len)) < 0) {
        printf("[TCP_SOCKET] Couldn't accept the client.\n");
        return false;
    }

    printf("[TCP_SOCKET] Client connected.\n");

    sock_state->interf = TCP;
    return true;
}

void close_tcp(SocketState* sock_state) {
    close(sock_state->client_fd);
    close(sock_state->server_fd);
    sock_state->interf = NONE;
}

int send_tcp(SocketState* socket, const void* buf, size_t len) {
    return write(socket->client_fd, buf, len);
}

int recv_tcp(SocketState* socket, void* buf, size_t len) {
    return recv(socket->server_fd, buf, len, MSG_WAITALL);
}