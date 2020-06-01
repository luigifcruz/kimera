#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "kimera/config.h"
#include "socket.h"

bool open_unix_client(SocketState*, State*);
bool open_unix_server(SocketState*, State*);
void close_unix(SocketState*);

#endif