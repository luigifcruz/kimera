#ifndef RECEIVER_H
#define RECEIVER_H

#include <stdio.h>

#include "kimera/utils.h"
#include "kimera/resampler.h"
#include "kimera/socket.h"
#include "kimera/decoder.h"
#include "kimera/loopback.h"
#include "kimera/display.h"
#include "kimera/router.h"

void receiver(State*, volatile sig_atomic_t*);

#endif