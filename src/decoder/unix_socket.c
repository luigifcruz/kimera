#include "unix_socket.h"

int open_unix_socket(char* socketaddr) {
    int socketfd;
    unix_addr server;

    if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("[UNIX_SOCKET] Couldn't open stream socket.\n");
        return -1;
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, socketaddr);

    if (connect(socketfd, (struct sockaddr *)&server, sizeof(unix_addr)) < 0) {
        printf("[UNIX_SOCKET] Couldn't connect to server.\n");
        return -1;
    }
    
    return socketfd;
}

void close_unix_socket(int socketfd) {
    close(socketfd);
}