#include "kimera/transport.hpp"

namespace Kimera {

Socket::Socket(State& state) : state(state), crypto(state), router(state) {}

Socket::~Socket() {
    switch (interf) {
        case Interfaces::UDP:
            CloseUDP();
            break;
        case Interfaces::TCP:
            CloseTCP();
            break;
        case Interfaces::UNIX:
            CloseUNIX();
            break;
        default:
            break;
    }

#ifdef KIMERA_WINDOWS
    WSACleanup();
#endif
}

bool Socket::LoadClient() {
#ifdef KIMERA_WINDOWS
    WSADATA wsd;
    WSAStartup(WINSOCK_VERSION, &wsd);
#endif

    if (CHECK(state.source, Interfaces::TCP))
        OpenTCPClient();

    if (CHECK(state.source, Interfaces::UDP))
        OpenUDPClient();

    if (CHECK(state.source, Interfaces::UNIX))
        OpenUNIXClient();

    if (interf == Interfaces::NONE) {
        printf("[SOCKET] Can't initiate socket client.\n");
        return false;
    }

    return true;
}

bool Socket::LoadServer() {
#ifdef KIMERA_WINDOWS
    WSADATA wsd;
    WSAStartup(WINSOCK_VERSION, &wsd);
#endif

    if (CHECK(state.sink, Interfaces::TCP))
        OpenTCPServer();

    if (CHECK(state.sink, Interfaces::UDP))
        OpenUDPServer();

    if (CHECK(state.sink, Interfaces::UNIX))
        OpenUNIXServer();

    if (interf == Interfaces::NONE) {
        printf("[SOCKET] Can't initiate socket server.\n");
        return false;
    }

    return true;
}

void Socket::Push(AVPacket* packet) {
    if (packet == NULL) return;
    while (router.Push(packet)) {
        void* buf  = router.BufferPtr();
        size_t len = router.BufferSize();

        switch (interf) {
            case Interfaces::UDP:  SendUDP(buf, len);  break;
            case Interfaces::TCP:  SendTCP(buf, len);  break;
            case Interfaces::UNIX: SendUNIX(buf, len); break;
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
            case Interfaces::UDP:  out = RecvUDP(buf, len);  break;
            case Interfaces::TCP:  out = RecvTCP(buf, len);  break;
            case Interfaces::UNIX: out = RecvUNIX(buf, len); break;
            default:   break;
        }

        AVPacket* packet = router.Pull(out);
        if (packet) return packet;
    }
}

} // namespace Kimera