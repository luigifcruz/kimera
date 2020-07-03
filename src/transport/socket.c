#include "kimera/transport.h"

SocketState* init_socket() {
    SocketState* state = malloc(sizeof(SocketState));
    state->router    = init_router();
    state->crypto    = NULL;
    state->packet    = NULL;
    state->client_in = NULL;
    state->server_in = NULL;
    state->client_un = NULL;
    state->server_un = NULL;
    return state;
}

void close_socket(SocketState* socket) {
    if (socket->interf != NONE)
        close_router(socket->router);

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

#ifdef KIMERA_WINDOWS
    WSACleanup();
#endif
}

bool open_socket_client(SocketState* socket, State* state) {
#ifdef KIMERA_WINDOWS
    WSADATA wsd;
    WSAStartup(WINSOCK_VERSION, &wsd);
#endif

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

    if (!start_router(socket->router, state))
        return false;

    socket->packet = socket->router->packet;
    return true;
}

bool open_socket_server(SocketState* socket, State* state) {
#ifdef KIMERA_WINDOWS
    WSADATA wsd;
    WSAStartup(WINSOCK_VERSION, &wsd);
#endif

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

    if (!start_router(socket->router, state))
        return false;

    socket->packet = socket->router->packet;
    return true;
}

void socket_send_packet(SocketState* socket, AVPacket* packet) {
    while (router_make_packet(socket->router, packet))
        socket_send_buffer(socket, socket->router->buffer, socket->router->packet_size);
}

int socket_recv_packet(SocketState* socket) {
    while (true) {
        size_t out = socket_recv_buffer(socket, socket->router->buffer, socket->router->packet_size);
        if (out < (size_t)socket->router->header_size) return false;
        if (router_parse_packet(socket->router)) return true;
    }   
}

int socket_send_buffer(SocketState* socket, const void* buf, size_t len) {
    switch (socket->interf) {
        case UDP:
            return send_udp(socket, buf, len);
        case TCP:
            return send_tcp(socket, buf, len);
        case UNIX:
            return send_unix(socket, buf, len);
        default:
            break;
    }
    return 0;
}

int socket_recv_buffer(SocketState* socket, void* buf, size_t len) {
    switch (socket->interf) {
        case UDP:
            return recv_udp(socket, buf, len);
        case TCP:
            return recv_tcp(socket, buf, len);
        case UNIX:
            return recv_unix(socket, buf, len);
        default:
            break;
    }
    return 0;
}
