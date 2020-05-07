#ifndef RECEIVER_H
#define RECEIVER_H

#include <stdio.h>

#include "utils.h"
#include "resampler.h"
#include "unix_socket.h"
#include "tcp_socket.h"
#include "codec/decoder.h"
#include "loopback/loopback.h"
#include "display.h"

void receiver(State*);

#endif