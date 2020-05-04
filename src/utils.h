#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static inline uint32_t
buffer_read32be(const uint8_t *buf) {
    return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
}

static inline uint64_t
buffer_read64be(const uint8_t *buf) {
    uint32_t msb = buffer_read32be(buf);
    uint32_t lsb = buffer_read32be(&buf[4]);
    return ((uint64_t) lsb << 32) | msb;
}

static int adb_open_tunnel(char* sname, char* domain) {
    char cmd[128];
    sprintf(cmd, "adb forward localfilesystem:%s localabstract:%s.%s",
            sname, domain, sname);
    printf("[ADB] %s\n", cmd);
    return system(cmd);
}

#endif
