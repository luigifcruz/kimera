#ifndef RECEIVER_H
#define RECEIVER_H

#include <stdio.h>
#include <libavutil/pixdesc.h>

#include "utils.h"
#include "resampler.h"
#include "unix_socket.h"
#include "tcp_socket.h"
#include "udp_socket.h"
#include "codec/decoder.h"
#include "loopback/loopback.h"
#include "display.h"
#include "router.h"

void receiver(State*, volatile sig_atomic_t*);

#endif