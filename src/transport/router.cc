#include "kimera/transport.hpp"

Router::Router(State& state) : state(state) {
    header_size = HEADER_SIZE;
    packet_size = state.packet_size;
    payload_size = state.packet_size - header_size;

    packet = (Packet*)malloc(sizeof(Packet));
    buffer = (char*)malloc(packet_size);

    if (packet == NULL || buffer == NULL) {
        printf("[ROUTER] Can't allocate router.\n");
        throw;
    }
}

Router::~Router() {
    if (packet->payload)
        free(packet->payload);
    if (buffer)
        free(buffer);
    if (packet)
        free(packet);
}


size_t Router::GetPacketSize(Packet* packet, size_t offset) {
    size_t pkt_size = payload_size;
    if ((offset + pkt_size) > packet->len) {
        pkt_size = packet->len - offset;
    }
    return pkt_size;
}

AVPacket* Router::Pull(size_t buf_len) {
    if (buf_len < (size_t)header_size) return NULL;

    uint64_t pts = buffer_read64be((uint8_t*)(buffer+0));
    uint32_t len = buffer_read32be((uint8_t*)(buffer+8));
    uint32_t i   = buffer_read32be((uint8_t*)(buffer+12));
    uint32_t n   = buffer_read32be((uint8_t*)(buffer+16));

    if (pts < packet->pts) return NULL;

    if (pts > packet->pts) {
        free(packet->payload);
        checksum = (uint32_t)0;
        packet->payload = (char*)malloc(len);
        if (packet->payload == NULL) {
            printf("[ROUTER] Can't allocate payload buffer.\n");
            return NULL;
        }
    }

    packet->pts = pts;
    packet->len = len;
    packet->i   = i;
    packet->n   = n;
    checksum   += 1;

    size_t offset = packet->i * payload_size;
    size_t size = GetPacketSize(packet, offset);
    memcpy(packet->payload + offset, (uint8_t*)(buffer+header_size), size);
    //printf("PTS: %llu LEN: %d I: %d N: %d\n", pts, len, i, n);

    if (checksum == packet->n) {
        AVPacket* packet = av_packet_alloc();
        av_init_packet(packet);
        packet->data = (uint8_t*)this->packet->payload;
        packet->size = this->packet->len;
        packet->pts = this->packet->pts;
        return packet;
    }

    return NULL;
}

bool Router::Push(AVPacket* packet) {
    bool is_new_frame = (this->packet->pts != (uint64_t)packet->pts);

    if (!is_new_frame && this->packet->i == this->packet->n - 1) {
        return false;
    }

    if (is_new_frame) {
        this->packet->pts = packet->pts;
        this->packet->len = packet->size;
        this->packet->i = 0;
        this->packet->n = (uint32_t)(packet->size / payload_size);
        this->packet->n += (uint32_t)((packet->size % payload_size) == 0 ? 0 : 1);
    } else {
        this->packet->i += 1;
    }

    size_t offset = 0;
    memcpy(buffer + offset, (char*)&this->packet->pts, sizeof(this->packet->pts));
    offset += sizeof(this->packet->pts);

    memcpy(buffer + offset, (char*)&this->packet->len, sizeof(this->packet->len));
    offset += sizeof(this->packet->len);

    memcpy(buffer + offset, (char*)&this->packet->i, sizeof(this->packet->i));
    offset += sizeof(this->packet->i);

    memcpy(buffer + offset, (char*)&this->packet->n, sizeof(this->packet->n));
    offset += sizeof(this->packet->n);

    size_t payload_offset = this->packet->i * this->payload_size;
    size_t size = GetPacketSize(this->packet, payload_offset);
    memcpy(buffer + offset, packet->data + payload_offset, size);

    return true;
}

char* Router::BufferPtr() {
    return buffer;
}

size_t Router::BufferSize() {
    return packet_size;
}
