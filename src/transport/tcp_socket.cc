#include "kimera/transport.hpp"

bool Socket::OpenTCPClient() {
    server_in = (socket_in*)malloc(sizeof(socket_in));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    memset(server_in, 0, sizeof(socket_in));

    server_in->sin_family = AF_INET;
    server_in->sin_port = htons(state.port);

    if (inet_pton(AF_INET, state.address.c_str(), &server_in->sin_addr) < 0) {
        printf("[TCP_SOCKET] Invalid address.\n");
        return false;
    }

    if (connect(server_fd, (socket_t*)server_in, sizeof(socket_in)) < 0) {
        printf("[TCP_SOCKET] Couldn't connect to server.\n");
        return false;
    }

    if (CHECK(state.pipe, Interfaces::CRYPTO)) {
        if (!crypto.Connect(server_fd)) return false;
    }

    interf = Interfaces::TCP;
    return true;
}

bool Socket::OpenTCPServer() {
    server_in = (socket_in*)malloc(sizeof(socket_in));
    client_in = (socket_in*)malloc(sizeof(socket_in));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    const int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
        printf("[TCP_SOCKET] Failed trying to reuse socket.\n");
    }

    memset(server_in, 0, sizeof(socket_in));
    memset(client_in, 0, sizeof(socket_in));

    server_in->sin_family = AF_INET;
    server_in->sin_addr.s_addr = htonl(INADDR_ANY);
    server_in->sin_port = htons(state.port);

    if (bind(server_fd, (socket_t*)server_in, sizeof(socket_in)) < 0) {
        printf("[TCP_SOCKET] Couldn't open server port.\n");
        return false;
    }

    if ((listen(server_fd, 1)) != 0) {
        printf("[TCP_SOCKET] Failed to listen.\n");
        return false;
    }

    printf("[TCP_SOCKET] Waiting client.\n");

    unsigned int len = sizeof(socket_in);
    if ((client_fd = accept(server_fd, (socket_t*)client_in, &len)) < 0) {
        printf("[TCP_SOCKET] Couldn't accept the client.\n");
        return false;
    }

    if (CHECK(state.pipe, Interfaces::CRYPTO)) {
        if (!crypto.Accept(client_fd)) return false;
    }

    printf("[TCP_SOCKET] Client connected.\n");

    interf = Interfaces::TCP;
    return true;
}

void Socket::CloseTCP() {
    close(client_fd);
    close(server_fd);
    interf = Interfaces::NONE;
}

int Socket::SendTCP(const void* buf, size_t len) {
    if (CHECK(state.pipe, Interfaces::CRYPTO))
        return crypto.Send(buf, len);
    return write(client_fd, buf, len);
}

int Socket::RecvTCP(void* buf, size_t len) {
    if (CHECK(state.pipe, Interfaces::CRYPTO))
        return crypto.Recv(buf, len);
    return recv(server_fd, buf, len, MSG_WAITALL);
}
