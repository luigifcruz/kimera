#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "kimera/transmitter.h"
#include "kimera/receiver.h"
#include "kimera/kimera.h"

volatile sig_atomic_t stop;

void inthand(int signum) {
    if (stop == 1) {
      exit(-1);
    }
    printf("Safely exiting, press Crtl-C to force shutdown.\n");
    stop = 1;
}

void print_version() {
    //printf("Kimera Version: %d.%d\n", kimera_VERSION_MAJOR, kimera_VERSION_MINOR);
    printf("AV Version: %s\n", av_version_info());
}

void print_help() {
    printf("Usage:\n   kimera [mode] [profile] [config]\n");
    printf("    - [profile] (required)  Name of the selected profile from the configuration file.\n");
    printf("    - [mode]    (required)  Operation Mode: Transmitter (tx) or Receiver (rx).\n");
    printf("    - [config]  (optional)  Path of the configuration file containing profiles.\n");
    printf("Example:\n   kimera tx rpi4_picam_v1 profiles.yml\n");
}

void print_io_list(Interfaces interfaces) {
    if (interfaces == NONE)
        printf(" NONE");
    if (interfaces & TCP)
        printf(" TCP");
    if (interfaces & UDP)
        printf(" UDP");
    if (interfaces & UNIX)
        printf(" UNIX");
    if (interfaces & STDOUT)
        printf(" STDOUT");
    if (interfaces & STDIN)
        printf(" STDIN");
    if (interfaces & DISPLAY)
        printf(" DISPLAY");
    if (interfaces & LOOPBACK)
        printf(" LOOPBACK");
}

void print_state(State* state) {
    printf(".   CURRENT STATE\n");
    printf("├── Dimensions: %dx%d\n", state->width, state->height);
    printf("├── Framerate:  %d FPS\n", state->framerate);
    printf("├── Bitrate:    %d bps\n", state->bitrate);
    printf("└── Packet Len: %d Bytes\n", state->packet_size);

    if (state->mode == TRANSMITTER) {
        printf("    .   TRANSMITTER\n");
        printf("    ├── Source: ");
        print_io_list(state->source);
        printf("\n");
        printf("    ├── Sink:   ");
        print_io_list(state->sink);
        printf("\n");
    }
    
    if (state->mode == RECEIVER) {
        printf("    .   RECEIVER\n");
        printf("    ├── Source: ");
        print_io_list(state->source);
        printf("\n");
        printf("    ├── Sink:   ");
        print_io_list(state->sink);
        printf("\n");
    }

    printf("    ├── Device:  %s\n", state->loopback);
    printf("    ├── Address: %s\n", state->address);
    printf("    ├── Port:    %d\n", state->port);
    printf("    └── Codec:   %s\n", state->codec);
}

int main(int argc, char *argv[]) {
    // Register signal handler.
    signal(SIGINT, inthand);

    // Declare Default Settings
    State* state = kimera_init();

    // Parse Command-Line Arguments
    if (argc < 2) {
        printf("Not enough arguments.\n");
        print_help();
        return -1;
    }

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        print_help();
        return 0;
    }

    if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
        print_version();
        return 0;
    }

    if (argc < 3) {
        printf("Not enough arguments.\n");
        print_help();
        return -1;
    }

    char* config_path = argv[2];

    if (!strcmp(argv[1], "tx")) {
        state->mode = TRANSMITTER;
        if (!kimera_load_state(state, config_path))
            return -1;
        transmitter(state, &stop);
        return 0;
    }
    
    if (!strcmp(argv[1], "rx")) {
        state->mode = RECEIVER;
        if (!kimera_load_state(state, config_path))
            return -1;
        receiver(state, &stop);
        return 0;
    }

    kimera_free(state);
    printf("Not such flag (%s).\n", argv[1]);
    return -1;
}
