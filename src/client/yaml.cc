#include "kimera/client.hpp"

bool Client::ParseConfigFile(char* path) {
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

    Mode mode;
    int direction = 0;
    int mapping_index = 0;
    int sequence_index = 0;

    do {
        if (!yaml_parser_parse(&parser, &event)) {
            printf("Error while trying to parse configuration file: %s\n", parser.problem);
            return false;
        }

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

                if (!strcmp((char*)event.data.scalar.value, "pipe")) {
                    direction = 3;
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

                    if (!strcmp((char*)event.data.scalar.value, "vert_shader"))
                        char_ptr = state->vert_shader;

                    if (!strcmp((char*)event.data.scalar.value, "frag_shader"))
                        char_ptr = state->frag_shader;

                    if (!strcmp((char*)event.data.scalar.value, "psk_key"))
                        char_ptr = state->psk_key;
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
                        Interfaces counter;

                        if (!strcmp((char*)event.data.scalar.value, "tcp"))
                            counter = TCP;

                        if (!strcmp((char*)event.data.scalar.value, "udp"))
                            counter = UDP;

                        if (!strcmp((char*)event.data.scalar.value, "unix"))
                            counter = UNIX;

                        if (!strcmp((char*)event.data.scalar.value, "loopback"))
                            counter = LOOPBACK;

                        if (!strcmp((char*)event.data.scalar.value, "display"))
                            counter = DISPLAY;

                        if (!strcmp((char*)event.data.scalar.value, "filter"))
                            counter = FILTER;

                        if (!strcmp((char*)event.data.scalar.value, "resample"))
                            counter = RESAMPLE;

                        if (!strcmp((char*)event.data.scalar.value, "gpu_resample"))
                            counter = GPU_RESAMPLE;

                        if (!strcmp((char*)event.data.scalar.value, "crypto"))
                            counter = CRYPTO;

                        if (direction == 1)
                            state->source = static_cast<Interfaces>(counter | static_cast<Interfaces>(state->source));
                        if (direction == 2)
                            state->sink = static_cast<Interfaces>(counter | static_cast<Interfaces>(state->sink));
                        if (direction == 3)
                            state->pipe = static_cast<Interfaces>(counter | static_cast<Interfaces>(state->pipe));
                    }
                }
                break;
            default: break;
        }

        if (event.type != YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
        }
    } while (event.type != YAML_STREAM_END_EVENT);

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(config_file);

    return true;
}
