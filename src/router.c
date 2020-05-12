#include "router.h"

bool start_router(RouterState* router) {
    router->packet = malloc(sizeof(Packet));
    router->buffer = malloc(MAX_SIZE);

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

void close_router(RouterState* router) {
    if (router->packet->payload != NULL)
        free(router->packet->payload);
    free(router->buffer);
    free(router->packet);
}

size_t get_packet_size(Packet* packet, size_t offset) {
    size_t pkt_size = MAX_PAYLOAD;
    if ((offset + pkt_size) > packet->len) {
        pkt_size = packet->len - offset;
    }
    return pkt_size;
}

bool recv_packet(RouterState* router, int fd) {
    char header[HEADER_SIZE];

    while (1) {
        size_t out = recv(fd, &header, HEADER_SIZE, MSG_WAITALL);
        if (out < HEADER_SIZE) return false;

        uint64_t pts = buffer_read64be((uint8_t*)header);
        uint32_t len = buffer_read32be((uint8_t*)&header[8]);
        uint32_t i   = buffer_read32be((uint8_t*)&header[12]);
        uint32_t n   = buffer_read32be((uint8_t*)&header[16]);

        if (pts < router->packet->pts) continue;
        
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

        size_t offset = router->packet->i * MAX_PAYLOAD;
        size_t pkt_size = get_packet_size(router->packet, offset);
        
        out = recv(fd, router->packet->payload + offset, pkt_size, MSG_WAITALL); 
        if (out < pkt_size) return false;
        if (router->packet->i == router->packet->n - 1) return true;
    }
}

bool make_packet(RouterState* router, AVPacket* packet, AVFrame* frame) {
    bool is_new_frame = (router->packet->pts != (uint64_t)frame->pts);

    if (!is_new_frame && router->packet->i == router->packet->n - 1) {
        free(router->packet->payload);
        router->packet->payload = NULL;
        return false;
    }

    if (is_new_frame) {
        router->packet->pts = frame->pts;
        router->packet->len = packet->size;
        router->packet->i = 0;
        router->packet->n = (uint32_t)(packet->size / MAX_PAYLOAD);
        router->packet->n += (uint32_t)((packet->size % MAX_PAYLOAD) == 0 ? 0 : 1);
    } else {
        router->packet->i += 1;
        assert(router->packet->i < router->packet->n);
    }

    size_t offset = router->packet->i * MAX_PAYLOAD;
    size_t pkt_size = get_packet_size(router->packet, offset);
    
    router->packet->payload = (char*)malloc(pkt_size);
    if (router->packet->payload == NULL) {
        printf("[ROUTER] Can't allocate payload buffer.\n");
        return false;
    }

    memcpy(router->packet->payload, packet->data + offset, pkt_size);

    return true;
}

void send_packet(RouterState* router, int fd) {
    size_t offset = 0;
    size_t pkt_size = get_packet_size(router->packet, router->packet->i * MAX_PAYLOAD);

    memcpy(router->buffer + offset, (char*)&router->packet->pts, sizeof(router->packet->pts));
    offset += sizeof(router->packet->pts);

    memcpy(router->buffer + offset, (char*)&router->packet->len, sizeof(router->packet->len));
    offset += sizeof(router->packet->len);

    memcpy(router->buffer + offset, (char*)&router->packet->i, sizeof(router->packet->i));
    offset += sizeof(router->packet->i);

    memcpy(router->buffer + offset, (char*)&router->packet->n, sizeof(router->packet->n));
    offset += sizeof(router->packet->n);

    memcpy(router->buffer + offset, router->packet->payload, pkt_size);
    offset += pkt_size;

    write(fd, router->buffer, offset);
}