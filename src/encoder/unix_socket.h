#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct sockaddr_un unix_addr;

int open_unix_socket(char*);
void close_unix_socket(int);

#endif