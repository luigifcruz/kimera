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

bool parse_config_file(State* state, char* key, char* path) {
    FILE* config_file;
    yaml_parser_t parser;
    yaml_token_t token;
    
    if ((config_file = fopen(path, "r")) == NULL) {
        printf("Configuration file not found.\n");
        return false;
    }

    if (!yaml_parser_initialize(&parser)) {
        printf("Failed to initialize YAML parser.\n");
        return false;
    }

    yaml_parser_set_input_file(&parser, config_file);

    // Parsing Logic Here

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    fclose(config_file);

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
    state->address      = DEFAULT_ADDRESS;
    state->codec        = DEFAULT_CODEC;
    state->loopback     = DEFAULT_LOOPBACK;
    state->in_format    = DEFAULT_FORMAT;
    state->out_format   = DEFAULT_FORMAT;
    state->framerate    = DEFAULT_FRAMERATE;

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

    if (argc < 4) {
        printf("Not enough arguments.\n");
        print_help();
        return -1;
    }

    char* config_key = argv[2];
    char* config_path = argv[3];

    if (!strcmp(argv[1], "tx")) {
        state->mode = TRANSMITTER;
        if (!parse_config_file(state, config_key, config_path))
            return -1;
        transmitter(state);
        return 0;
    }
    
    if (!strcmp(argv[1], "rx")) {
        state->mode = RECEIVER;
        if (!parse_config_file(state, config_key, config_path))
            return -1;
        receiver(state);
        return 0;
    }

    printf("Not such flag (%s)\n", argv[1]);
    return -1;
}
