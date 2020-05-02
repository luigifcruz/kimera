#include "tcp_socket.h"

int open_tcp_socket(char* socketaddr, char* server_ip, int server_port) {
    int socketfd;
    tcp_addr server;

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SOCKET] Couldn't open stream socket.\n");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server.sin_addr) < 0) { 
        printf("[TCP_SOCKET] Invalid address.\n"); 
        return -1; 
    } 

    if (connect(socketfd, (struct sockaddr *)&server, sizeof(tcp_addr)) < 0) {
        printf("[TCP_SOCKET] Couldn't connect to server.\n");
        return -1;
    }
    
    return socketfd;
}

void close_tcp_socket(int socketfd) {
    close(socketfd);
}