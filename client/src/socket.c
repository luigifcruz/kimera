#include "socket.h"

size_t socket_read_all(int socketfd, char* buf, size_t len) {
    return recv(socketfd, buf, len, MSG_WAITALL);
}

size_t socket_read(int socketfd, char* buf, size_t len) {
    return recv(socketfd, buf, len, MSG_DONTWAIT);
}

int open_socket(char* socketaddr) {
    int socketfd;
    saddr server;

    if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("Couldn't open stream socket.\n");
        return -1;
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, socketaddr);

    if (connect(socketfd, (struct sockaddr *)&server, sizeof(saddr)) < 0) {
        printf("Couldn't connect to server.\n");
        return -1;
    }
    
    return socketfd;
}

void close_socket(int socketfd) {
    close(socketfd);
}