#include "kimera/socket.h"

SocketState* alloc_socket() {
    SocketState* state = malloc(sizeof(SocketState));
    state->router    = alloc_router();
    state->packet    = NULL;
    state->client_in = NULL;
    state->server_in = NULL;
    state->client_un = NULL;
    state->server_un = NULL;
    return state;
}

inline void free_socket(SocketState* socket) {
    if (socket->interface != NONE)
        free_router(socket->router);

    switch (socket->interface) {
        case UDP:
            close_udp(socket);
            break;
        case TCP:
            close_tcp(socket);
            break;
        case UNIX:
            close_unix(socket);
            break;
        default: break;
    }

    free(socket);
}

inline bool open_socket_client(SocketState* socket, State* state) {
    if (state->source & TCP)
        open_tcp_client(socket, state);

    if (state->source & UDP)
        open_udp_client(socket, state);

    if (state->source & UNIX)
        open_unix_client(socket, state);

    if (socket->interface == NONE) {
        printf("[SOCKET] Can't initiate socket client.\n");
        return false;
    }

    if (!start_router(socket->router, state))
        return false;

    socket->packet = socket->router->packet;
    return true;
}

inline bool open_socket_server(SocketState* socket, State* state) {
    if (state->sink & TCP)
        open_tcp_server(socket, state);

    if (state->sink & UDP)
        open_udp_server(socket, state);

    if (state->sink & UNIX)
        open_unix_server(socket, state);

    if (socket->interface == NONE) {
        printf("[SOCKET] Can't initiate socket server.\n");
        return false;
    }

    if (!start_router(socket->router, state))
        return false;

    socket->packet = socket->router->packet;
    return true;
}

inline void socket_send_packet(SocketState* socket, AVPacket* packet) {
    while (router_make_packet(socket->router, packet))
        socket_send_buffer(socket, socket->router->buffer, socket->router->packet_size);
}

inline int socket_recv_packet(SocketState* socket) {
    while (1) {
        size_t out = socket_recv_buffer(socket, socket->router->buffer, socket->router->packet_size);
        if (out < (size_t)socket->router->header_size) return false;
        if (router_parse_packet(socket->router)) return true;
    }   
}

inline int socket_send_buffer(SocketState* socket, const void* buf, size_t len) {
    if (socket->interface & UNIX || socket->interface & TCP) {
        return write(socket->client_fd, buf, len);
    }

    if (socket->interface & UDP) {
        socklen_t slen = sizeof(*socket->server_in);
        return sendto(socket->server_fd, buf, len, 0, (socket_t*)socket->server_in, slen);
    }
    return 0;
}

inline int socket_recv_buffer(SocketState* socket, void* buf, size_t len) {
    if (socket->interface & UNIX || socket->interface & TCP) {
        return recv(socket->server_fd, buf, len, MSG_WAITALL);
    }
        
    if (socket->interface & UDP) {
        socklen_t slen = sizeof(*socket->client_in);
        return recvfrom(socket->server_fd, buf, len, MSG_WAITALL, (socket_t*)socket->client_in, &slen);
    }
    return 0;
}