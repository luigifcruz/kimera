#include "kimera/transport.hpp"

Socket::Socket(Kimera* state) : crypto(state), router(state) {
    this->state = state;
}

Socket::~Socket() {
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

void Socket::Push(AVPacket* packet) {
    while (router.Push(packet)) {
        void* buf  = router.BufferPtr();
        size_t len = router.BufferSize();

        switch (interf) {
            case UDP:  SendUDP(buf, len);  break;
            case TCP:  SendTCP(buf, len);  break;
            case UNIX: SendUNIX(buf, len); break;
            default:   break;
        }
    }
}

AVPacket* Socket::Pull() {
    while (true) {
        void*  buf = router.BufferPtr();
        size_t len = router.BufferSize();
        size_t out = 0;

        switch (interf) {
            case UDP:  out = RecvUDP(buf, len);  break;
            case TCP:  out = RecvTCP(buf, len);  break;
            case UNIX: out = RecvUNIX(buf, len); break;
            default:   break;
        }

        AVPacket* packet = router.Pull(out);
        if (packet) return packet;
    }
}
