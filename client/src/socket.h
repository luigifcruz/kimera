#ifndef SOCKET_H
#define SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct sockaddr_un saddr;

size_t socket_read_all(int, char*, size_t);
size_t socket_read(int, char*, size_t);

int socket_open(char*);
void socket_close(int);

#endif