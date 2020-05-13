#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <yaml.h>

#include "config.h"
#include "transmitter.h"
#include "receiver.h"

void inthand(int signum) {
    if (stop == 1) {
      exit(-1);
    }
    stop = 1;
}

void print_version() {
    printf("Kimera Version %d.%d\n", kimera_VERSION_MAJOR, kimera_VERSION_MINOR);
}

void print_help() {
    printf("Usage:\n   kimera [mode] [profile] [config]\n");
    printf("    - [profile] (required)  Name of the selected profile from the configuration file.\n");
    printf("    - [mode]    (required)  Operation Mode: Transmitter (tx) or Receiver (rx).\n");
    printf("    - [config]  (optional)  Path of the configuration file containing profiles.\n");
    printf("Example:\n   kimera tx rpi4_picam_v1 profiles.yml\n");
}

void print_state(State* state) {
    printf(".   CURRENT STATE\n");
    printf("├── Dimensions: %dx%d\n", state->width, state->height);
    printf("├── Framerate:  %d FPS\n", state->framerate);
    printf("└── Bitrate:    %d bps\n", state->bitrate);

    if (state->mode == TRANSMITTER) {
        printf("    .   TRANSMITTER\n");
        printf("    ├── Source: ");
        if (state->source & LOOPBACK)
            printf(" LOOPBACK");
        printf("\n");
        printf("    ├── Sink:   ");
        if (state->sink & TCP)
            printf(" TCP");
        if (state->sink & UNIX)
            printf(" UNIX");
        if (state->sink & STDOUT)
            printf(" STDOUT");
        if (state->sink & DISPLAY)
            printf(" DISPLAY");
        if (state->sink & LOOPBACK)
            printf(" LOOPBACK");
        printf("\n");
    }
    
    if (state->mode == RECEIVER) {
        printf("    .   RECEIVER\n");
        printf("    ├── Source: ");
        if (state->source & TCP)
            printf(" TCP");
        if (state->source & UNIX)
            printf(" UNIX");
        printf("\n");
        printf("    ├── Sink:   ");
        if (state->sink & STDOUT)
            printf(" STDOUT");
        if (state->sink & DISPLAY)
            printf(" DISPLAY");
        if (state->sink & LOOPBACK)
            printf(" LOOPBACK");
        printf("\n");
    }

    printf("    ├── Device:  %s\n", state->loopback);
    printf("    ├── Address: %s\n", state->address);
    printf("    ├── Port:    %d\n", state->port);
    printf("    └── Codec:   %s\n", state->codec);
}

bool parse_config_file(State* state, char* path) {
    FILE* config_file;
    yaml_parser_t parser;
    yaml_event_t event;
    
    if ((config_file = fopen(path, "r")) == NULL) {
        printf("Configuration file not found.\n");
        return false;
    }

    if (!yaml_parser_initialize(&parser)) {
        printf("Failed to initialize YAML parser.\n");
        return false;
    }

    yaml_parser_set_input_file(&parser, config_file);

    int* int_ptr = NULL;
    char* char_ptr = NULL;

    Mode mode = 0;
    int direction = 0;
    int mapping_index = 0;
    int sequence_index = 0;

    // God dammit, Who Made This Library?
    while (event.type != YAML_STREAM_END_EVENT) {

        yaml_parser_parse(&parser, &event);

        switch(event.type) { 
            case YAML_MAPPING_START_EVENT: mapping_index += 1; break;
            case YAML_MAPPING_END_EVENT: mapping_index -= 1; break;
            case YAML_SEQUENCE_START_EVENT: sequence_index += 1; break;
            case YAML_SEQUENCE_END_EVENT: sequence_index -= 1; break;
            case YAML_SCALAR_EVENT:
                // Write Parameters
                if (int_ptr != NULL) {
                    *int_ptr = atoi((char*)event.data.scalar.value);
                    int_ptr = NULL;
                    break;
                }

                if (char_ptr != NULL) {
                    strcpy(char_ptr, (char*)event.data.scalar.value);
                    char_ptr = NULL;
                    break;
                }

                // Listen For Mapping Keys
                if (!strcmp((char*)event.data.scalar.value, "transmitter")) {
                    mode = TRANSMITTER;
                    break;
                }

                if (!strcmp((char*)event.data.scalar.value, "receiver")) {
                    mode = RECEIVER;
                    break;
                }

                if (!strcmp((char*)event.data.scalar.value, "source")) {
                    direction = 1;
                    break;
                }

                if (!strcmp((char*)event.data.scalar.value, "sink")) {
                    direction = 2;
                    break;
                }

                // Register Root Parameters
                if (mapping_index == 1) {
                    if (!strcmp((char*)event.data.scalar.value, "width"))
                        int_ptr = &state->width;

                    if (!strcmp((char*)event.data.scalar.value, "height"))
                        int_ptr = &state->height;

                    if (!strcmp((char*)event.data.scalar.value, "framerate"))
                        int_ptr = &state->framerate;

                    if (!strcmp((char*)event.data.scalar.value, "bitrate"))
                        int_ptr = &state->bitrate;
                }

                // Register Secondary Parameters
                if (mode == state->mode && mapping_index == 2) {
                    if (!strcmp((char*)event.data.scalar.value, "address"))
                        char_ptr = state->address;

                    if (!strcmp((char*)event.data.scalar.value, "device"))
                        char_ptr = state->loopback;

                    if (!strcmp((char*)event.data.scalar.value, "codec"))
                        char_ptr = state->codec;
                    
                    if (!strcmp((char*)event.data.scalar.value, "port"))
                        int_ptr = &state->port;

                    if (sequence_index == 1) {
                        int counter = 0;

                        if (!strcmp((char*)event.data.scalar.value, "tcp"))
                            counter += TCP;

                        if (!strcmp((char*)event.data.scalar.value, "unix"))
                            counter += UNIX;
                        
                        if (!strcmp((char*)event.data.scalar.value, "loopback"))
                            counter += LOOPBACK;

                        if (!strcmp((char*)event.data.scalar.value, "display"))
                            counter += DISPLAY;

                        if (!strcmp((char*)event.data.scalar.value, "stdout"))
                            counter += STDOUT;
                        
                        if (direction == 1)
                            state->source += counter;
                        if (direction == 2)
                            state->sink += counter;
                    }
                }
                break;
            default: break;
        }

        if (event.type != YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
        } 
    }

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(config_file);
    print_state(state);

    return true;
}

int main(int argc, char *argv[]) {
    // Register signal handler.
    signal(SIGINT, inthand);

    // Declare Default Settings
    State *state = malloc(sizeof(State));

    state->width        = DEFAULT_WIDTH;
    state->height       = DEFAULT_HEIGHT;
    state->bitrate      = DEFAULT_BITRATE;
    state->port         = DEFAULT_PORT;
    state->in_format    = DEFAULT_FORMAT;
    state->out_format   = DEFAULT_FORMAT;
    state->framerate    = DEFAULT_FRAMERATE;
    state->sink         = 0;
    state->source       = 0;

    state->loopback = malloc(64);
    state->address = malloc(64);
    state->codec = malloc(64);

    strcpy(state->loopback, DEFAULT_LOOPBACK);
    strcpy(state->address, DEFAULT_ADDRESS);
    strcpy(state->codec, DEFAULT_CODEC);

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
        if (!parse_config_file(state, config_path))
            return -1;
        transmitter(state);
        return 0;
    }
    
    if (!strcmp(argv[1], "rx")) {
        state->mode = RECEIVER;
        if (!parse_config_file(state, config_path))
            return -1;
        receiver(state);
        return 0;
    }

    printf("Not such flag (%s).\n", argv[1]);
    return -1;
}
