#include "socket.h"

int send_socket(SocketState* socket, const void* buf, size_t len) {
    if (socket->interface & UNIX || socket->interface & TCP)
        return write(socket->client_fd, buf, len);
    if (socket->interface & UDP)
        return sendto(
            socket->server_fd, buf, len, 0,
            (socket_t*)socket->server_in, sizeof(*socket->server_in));
    return 0;
}

int recv_socket(SocketState* socket, void* buf, size_t len) {
    if (socket->interface & UNIX || socket->interface & TCP)
        return recv(socket->server_fd, buf, len, MSG_WAITALL);
    if (socket->interface & UDP) {
        socklen_t slen = sizeof(*socket->client_in);
        return recvfrom(
            socket->server_fd, buf, len, MSG_WAITALL,
            (socket_t*)socket->client_in, &slen);
    }
    return 0;
}