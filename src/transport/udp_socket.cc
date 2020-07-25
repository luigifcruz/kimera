#include "kimera/transport.hpp"

namespace Kimera {

bool Socket::OpenUDPClient() {
    server_in = (socket_in*)malloc(sizeof(socket_in));
    client_in = (socket_in*)malloc(sizeof(socket_in));

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("[UDP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    memset(server_in, 0, sizeof(socket_in));
    memset(client_in, 0, sizeof(socket_in));

    server_in->sin_family = AF_INET;
    server_in->sin_addr.s_addr = INADDR_ANY;
    server_in->sin_port = htons(state.port);

    int n = 1024 * 1024;
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) == -1) {
        printf("[UDP_SOCKET] Couldn't allocate bigger UDP buffer. This might result in problems.\n");
    }

    if (bind(server_fd, (socket_t*)server_in, sizeof(socket_in)) < 0) {
        printf("[UDP_SOCKET] Couldn't bind with server port.\n");
        return false;
    }

    interf = Interfaces::UDP;
    return true;
}

bool Socket::OpenUDPServer() {
    server_in = (socket_in*)malloc(sizeof(socket_in));

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("[UDP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    memset(server_in, 0, sizeof(socket_in));

    server_in->sin_family = AF_INET;
    server_in->sin_port = htons(state.port);
    server_in->sin_addr.s_addr = inet_addr(state.address.c_str());

    interf = Interfaces::UDP;
    return true;
}

void Socket::CloseUDP() {
    close(client_fd);
    close(server_fd);
    interf = Interfaces::NONE;
}

int Socket::SendUDP(const void* buf, size_t len) {
    socklen_t slen = sizeof(*server_in);
    return sendto(server_fd, buf, len, 0, (socket_t*)server_in, slen);
}

int Socket::RecvUDP(void* buf, size_t len) {
    socklen_t slen = sizeof(*client_in);
    return recvfrom(server_fd, buf, len, MSG_WAITALL, (socket_t*)client_in, &slen);
}

} // namespace Kimera