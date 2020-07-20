#ifndef CONFIG_H
#define CONFIG_H

extern "C" {
#include <bits/stdint-uintn.h>
#include <libavcodec/avcodec.h>
#include <signal.h>
#include <stdbool.h>
}

//
// CMake Defined Settings
//

/* #undef KIMERA_WINDOWS */
/* #undef KIMERA_MACOS */
#define KIMERA_LINUX

#define KIMERA_VERSION_MAJOR 1
#define KIMERA_VERSION_MINOR 0

//
// Default Settings
//

#define HEADER_SIZE          20
#define KIMERA_PSK_IDENTITY  "KIMERA_PSK_IDENTITY"
#define MAX_KEY_LEN          256
#define DEFAULT_KEY_LEN      64

#define DEFAULT_PACKET_SIZE  1020
#define DEFAULT_PORT         8080
#define DEFAULT_ADDRESS      "0.0.0.0"
#define DEFAULT_LOOPBACK     "/dev/video0"
#define DEFAULT_FRAMERATE    25
#define DEFAULT_WIDTH        1280
#define DEFAULT_HEIGHT       720
#define DEFAULT_BITRATE      3000000
#define DEFAULT_CODEC        "libx264"
#define DEFAULT_FORMAT       AV_PIX_FMT_YUV420P

char* empty_string(size_t len);
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }

typedef enum {
    NONE         = 0 << 0,
    // Source/Sink Options
    UNIX         = 1 << 0,
    TCP          = 1 << 1,
    UDP          = 1 << 2,
    DISPLAY      = 1 << 3,
    LOOPBACK     = 1 << 4,
    // Pipe Options
    FILTER       = 1 << 5,
    RESAMPLE     = 1 << 6,
    GPU_RESAMPLE = 1 << 7,
    CRYPTO       = 1 << 8,
} Interfaces;

typedef enum {
    TRANSMITTER = 1 << 0,
    RECEIVER    = 1 << 1,
} Mode;

typedef struct {
    uint64_t pts;
    uint32_t len;
    uint32_t i;
    uint32_t n;
    char* payload;
} Packet;

class Kimera {
public:
    Kimera();
    ~Kimera();

    int width       = DEFAULT_WIDTH;
    int height      = DEFAULT_HEIGHT;
    int bitrate     = DEFAULT_BITRATE;
    int framerate   = DEFAULT_FRAMERATE;
    int port        = DEFAULT_PORT;
    int packet_size = DEFAULT_PACKET_SIZE;

    char* vert_shader = empty_string(256);
    char* frag_shader = empty_string(256);
    char* psk_key     = empty_string(256);
    char* codec       = empty_string(64);
    char* address     = empty_string(64);
    char* loopback    = empty_string(64);

    enum AVPixelFormat in_format  = DEFAULT_FORMAT;
    enum AVPixelFormat out_format = DEFAULT_FORMAT;

    Mode mode;

    Interfaces source = NONE;
    Interfaces pipe   = NONE;
    Interfaces sink   = NONE;
};

#endif
