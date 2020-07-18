#include "kimera/transport.h"

RouterState* init_router() {
    RouterState* state = malloc(sizeof(RouterState));
    state->packet = NULL;
    state->buffer = NULL;
    return state;
}

void close_router(RouterState* router) {
    if (router->packet->payload)
        free(router->packet->payload);
    if (router->buffer)
        free(router->buffer);
    if (router->packet)
        free(router->packet);
    free(router);
}

bool start_router(RouterState* router, State* state) {
    router->header_size = HEADER_SIZE;
    router->packet_size = state->packet_size;
    router->payload_size = state->packet_size - router->header_size;

    router->packet = malloc(sizeof(Packet));
    router->buffer = malloc(router->packet_size);

    if (router->packet == NULL || router->buffer == NULL) {
        printf("[ROUTER] Can't allocate router.\n");
        return false;
    }

    router->packet->pts = (uint64_t)0;
    router->packet->len = (uint32_t)0;
    router->packet->i   = (uint32_t)0;
    router->packet->n   = (uint32_t)0;
    router->checksum    = (uint32_t)0;

    router->packet->payload = NULL;
    return true;
}

size_t get_packet_size(RouterState* router, Packet* packet, size_t offset) {
    size_t pkt_size = router->payload_size;
    if ((offset + pkt_size) > packet->len) {
        pkt_size = packet->len - offset;
    }
    return pkt_size;
}

bool router_parse_packet(RouterState* router) {
    uint64_t pts = buffer_read64be((uint8_t*)(router->buffer+0));
    uint32_t len = buffer_read32be((uint8_t*)(router->buffer+8));
    uint32_t i   = buffer_read32be((uint8_t*)(router->buffer+12));
    uint32_t n   = buffer_read32be((uint8_t*)(router->buffer+16));

    if (pts < router->packet->pts)
        return false;

    if (pts > router->packet->pts) {
        free(router->packet->payload);
        router->checksum = (uint32_t)0;
        router->packet->payload = malloc(len);
        if (router->packet->payload == NULL) {
            printf("[ROUTER] Can't allocate payload buffer.\n");
            return false;
        }
    }

    router->packet->pts = pts;
    router->packet->len = len;
    router->packet->i   = i;
    router->packet->n   = n;
    router->checksum   += 1;

    size_t offset = router->packet->i * router->payload_size;
    size_t size = get_packet_size(router, router->packet, offset);
    memcpy(router->packet->payload + offset, (uint8_t*)(router->buffer+router->header_size), size);
    //printf("PTS: %llu LEN: %d I: %d N: %d\n", pts, len, i, n);

    if (router->checksum == router->packet->n)
        return true;

    return false;
}

bool router_make_packet(RouterState* router, AVPacket* packet) {
    bool is_new_frame = (router->packet->pts != (uint64_t)packet->pts);

    if (!is_new_frame && router->packet->i == router->packet->n - 1) {
        return false;
    }

    if (is_new_frame) {
        router->packet->pts = packet->pts;
        router->packet->len = packet->size;
        router->packet->i = 0;
        router->packet->n = (uint32_t)(packet->size / router->payload_size);
        router->packet->n += (uint32_t)((packet->size % router->payload_size) == 0 ? 0 : 1);
    } else {
        router->packet->i += 1;
    }

    size_t offset = 0;
    memcpy(router->buffer + offset, (char*)&router->packet->pts, sizeof(router->packet->pts));
    offset += sizeof(router->packet->pts);

    memcpy(router->buffer + offset, (char*)&router->packet->len, sizeof(router->packet->len));
    offset += sizeof(router->packet->len);

    memcpy(router->buffer + offset, (char*)&router->packet->i, sizeof(router->packet->i));
    offset += sizeof(router->packet->i);

    memcpy(router->buffer + offset, (char*)&router->packet->n, sizeof(router->packet->n));
    offset += sizeof(router->packet->n);

    size_t payload_offset = router->packet->i * router->payload_size;
    size_t size = get_packet_size(router, router->packet, payload_offset);
    memcpy(router->buffer + offset, packet->data + payload_offset, size);

    return true;
}
