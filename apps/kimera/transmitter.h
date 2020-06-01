#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <stdio.h>

#include "kimera/utils.h"
#include "kimera/socket.h"
#include "kimera/encoder.h"
#include "kimera/resampler.h"
#include "kimera/loopback.h"
#include "kimera/display.h"
#include "kimera/router.h"

void transmitter(State*, volatile sig_atomic_t*);

#endif