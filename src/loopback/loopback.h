#ifndef LOOPBACK_H
#define LOOPBACK_H

#ifdef __unix__
#include "linux/loopback.h"
#elif __APPLE__
#include "macos/loopback.h"
#endif

#endif