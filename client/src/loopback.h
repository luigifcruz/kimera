#ifdef __unix__
#include "linux/loopback.h"
#include "linux/loopback.c"
#elif __APPLE__
#include "macos/loopback.h"
#include "macos/loopback.c"
#endif