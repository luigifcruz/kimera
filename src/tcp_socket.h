#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 

#include "config.h"

typedef struct sockaddr_in tcp_addr;

int open_tcp_socket(State*);
void close_tcp_socket(int);

#endif