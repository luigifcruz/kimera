#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <stdio.h>
#include <libavutil/pixdesc.h>

#include "utils.h"
#include "unix_socket.h"
#include "tcp_socket.h"
#include "codec/encoder.h"
#include "resampler.h"
#include "loopback/loopback.h"
#include "display.h"

void transmitter(State*);

#endif