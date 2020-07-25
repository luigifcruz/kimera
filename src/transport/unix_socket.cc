#include "kimera/transport.hpp"

bool Socket::OpenUNIXClient() {
    server_un = (socket_un*)malloc(sizeof(socket_un));

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("[UNIX_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    server_un->sun_family = AF_UNIX;
    strcpy(server_un->sun_path, state.address.c_str());

    if (connect(server_fd, (socket_t*)server_un, sizeof(socket_un)) < 0) {
        printf("[UNIX_SOCKET] Couldn't connect to server.\n");
        return false;
    }

    interf = Interfaces::UNIX;
    return true;
}

bool Socket::OpenUNIXServer() {
    server_un = (socket_un*)malloc(sizeof(socket_un));
    client_un = (socket_un*)malloc(sizeof(socket_un));

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("[UNIX_SOCKET] Couldn't open server socket.\n");
        return false;
    }

    server_un->sun_family = AF_UNIX;
    strcpy(server_un->sun_path, state.address.c_str());
    unlink(server_un->sun_path);

    if (bind(server_fd, (socket_t*)server_un, sizeof(socket_un)) < 0) {
        printf("[UNIX_SOCKET] Couldn't create socket server.\n");
        return false;
    }

    if (listen(server_fd, 5) < 0) {
        printf("[UNIX_SOCKET] Couldn't listen socket.\n");
        return false;
    }

    printf("[UNIX_SOCKET] Waiting client.\n");

    unsigned int len = sizeof(socket_un);
    if ((client_fd = accept(server_fd, (socket_t*)client_un, &len)) < 0) {
        printf("[UNIX_SOCKET] Couldn't accept the client.\n");
        return false;
    }

    printf("[UNIX_SOCKET] Client connected.\n");

    interf = Interfaces::UNIX;
    return true;
}

void Socket::CloseUNIX() {
    close(server_fd);
    close(client_fd);
    interf = Interfaces::NONE;
}

int Socket::SendUNIX(const void* buf, size_t len) {
    return write(client_fd, buf, len);
}

int Socket::RecvUNIX(void* buf, size_t len) {
    return recv(server_fd, buf, len, MSG_WAITALL);
}
