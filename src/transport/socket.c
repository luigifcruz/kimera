#include "kimera/transport.h"

bool open_socket_client(SocketState* socket, State* state) {
    if (state->source & TCP)
        open_tcp_client(socket, state);

    if (state->source & UDP)
        open_udp_client(socket, state);

    if (state->source & UNIX)
        open_unix_client(socket, state);

    if (socket->interf == NONE) {
        printf("[SOCKET] Can't initiate socket client.\n");
        return false;
    }

    if (!start_router(&socket->router, state))
        return false;

    socket->packet = socket->router.packet;
    return true;
}

bool open_socket_server(SocketState* socket, State* state) {
    if (state->sink & TCP)
        open_tcp_server(socket, state);

    if (state->sink & UDP)
        open_udp_server(socket, state);

    if (state->sink & UNIX)
        open_unix_server(socket, state);

    if (socket->interf == NONE) {
        printf("[SOCKET] Can't initiate socket server.\n");
        return false;
    }

    if (!start_router(&socket->router, state))
        return false;

    socket->packet = socket->router.packet;
    return true;
}

void close_socket(SocketState* socket) {
    if (socket->interf != NONE)
        close_router(&socket->router);

    switch (socket->interf) {
        case UDP:
            close_udp(socket);
            break;
        case TCP:
            close_tcp(socket);
            break;
        case UNIX:
            close_unix(socket);
            break;
        default:
            break;
    }
}

void socket_send_packet(SocketState* socket, AVPacket* packet) {
    while (router_make_packet(&socket->router, packet))
        socket_send_buffer(socket, socket->router.buffer, socket->router.packet_size);
}

int socket_recv_packet(SocketState* socket) {
    while (1) {
        size_t out = socket_recv_buffer(socket, socket->router.buffer, socket->router.packet_size);
        if (out < (size_t)socket->router.header_size) return false;
        if (router_parse_packet(&socket->router)) return true;
    }   
}

int socket_send_buffer(SocketState* socket, const void* buf, size_t len) {
    if (socket->interf & UNIX || socket->interf & TCP) {
        return write(socket->client_fd, buf, len);
    }

    if (socket->interf & UDP) {
        socklen_t slen = sizeof(*socket->server_in);
        return sendto(socket->server_fd, buf, len, 0, (socket_t*)socket->server_in, slen);
    }
    return 0;
}

int socket_recv_buffer(SocketState* socket, void* buf, size_t len) {
    if (socket->interf & UNIX || socket->interf & TCP) {
        return recv(socket->server_fd, buf, len, MSG_WAITALL);
    }
        
    if (socket->interf & UDP) {
        socklen_t slen = sizeof(*socket->client_in);
        return recvfrom(socket->server_fd, buf, len, MSG_WAITALL, (socket_t*)socket->client_in, &slen);
    }
    return 0;
}