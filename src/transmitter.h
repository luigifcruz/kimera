#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <stdio.h>

#include "utils.h"
#include "unix_socket.h"
#include "tcp_socket.h"
#include "udp_socket.h"
#include "codec/encoder.h"
#include "resampler.h"
#include "loopback/loopback.h"
#include "display.h"
#include "router.h"

void transmitter(State*, volatile sig_atomic_t*);

#endif