#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdint.h>

static inline uint32_t
buffer_read32be(const char *buf) {
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

static inline uint64_t
buffer_read64be(const char *buf) {
    uint32_t msb = buffer_read32be(buf);
    uint32_t lsb = buffer_read32be(&buf[4]);
    return ((uint64_t) msb << 32) | lsb;
}

int adb_open_tunnel(char* sname, char* domain) {
    char cmd[128];
    sprintf(cmd, "adb forward localfilesystem:%s localabstract:%s.%s",
            sname, domain, sname);
    printf("[ADB] %s\n", cmd);
    return system(cmd);
}

#endif

