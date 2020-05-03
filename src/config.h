#ifndef CONFIG_H
#define CONFIG_H

#include <libavcodec/avcodec.h>
#include <signal.h>

//
// Default Settings
//

#define HEADER_SIZE         12
#define NO_PTS              UINT64_C(-1)

#define DEFAULT_PORT        8080
#define DEFAULT_ADDRESS     "127.0.0.1"
#define DEFAULT_LOOPBACK    "/dev/video0"

#define DEFAULT_FRAMERATE   30
#define DEFAULT_WIDTH       1920
#define DEFAULT_HEIGHT      1080
#define DEFAULT_BITRATE     5000000
#define DEFAULT_CODEC       "hevc"
#define DEFAULT_FORMAT      AV_PIX_FMT_YUV420P

//
// Variables
//

volatile sig_atomic_t stop;

typedef enum {
    UNIX,
    TCP,
    STDOUT,
    DISPLAY,
    LOOPBACK
} Interfaces;

typedef struct {
    int width;
    int height;
    char* codec;
    int bitrate;
    int framerate;
    int frame_size;
    enum AVPixelFormat format;

    int port;
    char* address;
    char* loopback;

    Interfaces sink;
    Interfaces source;
} State;

//
// Methods
//


#endif