#include "kimera/kimera.h"

State* kimera_init() {
    State *state = malloc(sizeof(State));

    state->width        = DEFAULT_WIDTH;
    state->height       = DEFAULT_HEIGHT;
    state->bitrate      = DEFAULT_BITRATE;
    state->port         = DEFAULT_PORT;
    state->in_format    = DEFAULT_FORMAT;
    state->out_format   = DEFAULT_FORMAT;
    state->framerate    = DEFAULT_FRAMERATE;
    state->packet_size  = DEFAULT_PACKET_SIZE;
    state->sink         = 0;
    state->source       = 0;

    state->loopback = malloc(64);
    state->address = malloc(64);
    state->codec = malloc(64);

    strcpy(state->loopback, DEFAULT_LOOPBACK);
    strcpy(state->address, DEFAULT_ADDRESS);
    strcpy(state->codec, DEFAULT_CODEC);

    return state;
}

void kimera_free(State* state) {
    if (!state)
        return;

    free(state->loopback);
    free(state->address);
    free(state->codec);
    free(state);
}

bool kimera_load_state(State* state, char* path) {
    FILE* config_file;
    yaml_parser_t parser;
    yaml_event_t event;
    
    if ((config_file = fopen(path, "r")) == NULL) {
        printf("[KIMERA] Configuration file not found.\n");
        return false;
    }

    if (!yaml_parser_initialize(&parser)) {
        printf("[KIMERA] Failed to initialize YAML parser.\n");
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

                    if (!strcmp((char*)event.data.scalar.value, "packet_size"))
                        int_ptr = &state->packet_size;
                    
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

                        if (!strcmp((char*)event.data.scalar.value, "udp"))
                            counter += UDP;

                        if (!strcmp((char*)event.data.scalar.value, "unix"))
                            counter += UNIX;
                        
                        if (!strcmp((char*)event.data.scalar.value, "loopback"))
                            counter += LOOPBACK;

                        if (!strcmp((char*)event.data.scalar.value, "display"))
                            counter += DISPLAY;

                        if (!strcmp((char*)event.data.scalar.value, "stdout"))
                            counter += STDOUT;
                        
                        if (!strcmp((char*)event.data.scalar.value, "stdin"))
                            counter += STDIN;
                        
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

    return true;
}