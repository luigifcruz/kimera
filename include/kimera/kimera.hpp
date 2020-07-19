#ifndef CONFIG_H
#define CONFIG_H

#include <libavcodec/avcodec.h>
#include <signal.h>
#include <stdbool.h>

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

class State {
public:
    int width;
    int height;
    int bitrate;
    int framerate;
    int port;
    int packet_size;

    char* codec;
    char* vert_shader;
    char* frag_shader;
    char* psk_key;
    char* address;
    char* loopback;

    enum AVPixelFormat in_format;
    enum AVPixelFormat out_format;

    Mode mode;

    Interfaces source;
    Interfaces pipe;
    Interfaces sink;

    State();
    ~State();
};

#endif
