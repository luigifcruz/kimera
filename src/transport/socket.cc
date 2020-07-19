#include "kimera/transport.hpp"

Socket::Socket(Kimera* state) : crypto(state), router(state) {
    this->state = state;
}

Socket::~Socket() {
    if (interf != NONE)
        router.~Router();

    switch (interf) {
        case UDP:
            CloseUDP();
            break;
        case TCP:
            CloseTCP();
            break;
        case UNIX:
            CloseUNIX();
            break;
        default:
            break;
    }

#ifdef KIMERA_WINDOWS
    WSACleanup();
#endif
}

bool Socket::OpenClient() {
#ifdef KIMERA_WINDOWS
    WSADATA wsd;
    WSAStartup(WINSOCK_VERSION, &wsd);
#endif

    if (state->source & TCP)
        OpenTCPClient();

    if (state->source & UDP)
        OpenUDPClient();

    if (state->source & UNIX)
        OpenUNIXClient();

    if (interf == NONE) {
        printf("[SOCKET] Can't initiate socket client.\n");
        return false;
    }

    return true;
}

bool Socket::OpenServer() {
#ifdef KIMERA_WINDOWS
    WSADATA wsd;
    WSAStartup(WINSOCK_VERSION, &wsd);
#endif

    if (state->sink & TCP)
        OpenTCPServer();

    if (state->sink & UDP)
        OpenUDPServer();

    if (state->sink & UNIX)
        OpenUNIXServer();

    if (interf == NONE) {
        printf("[SOCKET] Can't initiate socket server.\n");
        return false;
    }

    return true;
}

void Socket::SendPacket(AVPacket* packet) {
    while (router.MakePacket(packet))
        SendBuffer(router.GetBuffer(), router.GetPacketSize());
}

AVPacket* Socket::RecvPacket() {
    while (true) {
        size_t out = SendBuffer(router.GetBuffer(), router.GetPacketSize());
        if (out < (size_t)router.GetHeaderSize()) return NULL;
        if (router.ParsePacket()) return router.GetPacket();
    }
}

int Socket::SendBuffer(const void* buf, size_t len) {
    switch (interf) {
        case UDP:
            return SendUDP(buf, len);
        case TCP:
            return SendTCP(buf, len);
        case UNIX:
            return SendUNIX(buf, len);
        default:
            break;
    }
    return 0;
}

int Socket::RecvBuffer(void* buf, size_t len) {
    switch (interf) {
        case UDP:
            return RecvUDP(buf, len);
        case TCP:
            return RecvTCP(buf, len);
        case UNIX:
            return RecvUNIX(buf, len);
        default:
            break;
    }
    return 0;
}
