#include "tcp_socket.h"

bool open_tcp_client(TCPSocketState* tcp, State* state) {
    socket_in server;

    if ((tcp->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(state->port);

    if (inet_pton(AF_INET, state->address, &server.sin_addr) < 0) { 
        printf("[TCP_SOCKET] Invalid address.\n"); 
        return false;
    } 

    if (connect(tcp->server_fd, (socket_t*)&server, sizeof(socket_in)) < 0) {
        printf("[TCP_SOCKET] Couldn't connect to server.\n");
        return false;
    }

    return true;
}

bool open_tcp_server(TCPSocketState* tcp, State* state) {
    socket_in server, client;

    if ((tcp->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }
    bzero(&server, sizeof(server)); 
  
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(state->port); 

    if (bind(tcp->server_fd, (socket_t*)&server, sizeof(socket_in)) < 0) {
        printf("[TCP_SOCKET] Couldn't connect to any client.\n");
        return false;
    }

    if ((listen(tcp->server_fd, 5)) != 0) { 
        printf("[TCP_SOCKET] Failed to listen.\n");
        return false;
    }

    printf("[TCP_SOCKET] Waiting client.\n");

    unsigned int len = sizeof(client); 
    if ((tcp->client_fd = accept(tcp->server_fd, (socket_t*)&client, &len)) < 0) {
        printf("[TCP_SOCKET] Couldn't accept the client.\n");
        return false;
    }

    return true;
}

void close_tcp(TCPSocketState* tcp) {
    close(tcp->client_fd);
    close(tcp->server_fd);
}