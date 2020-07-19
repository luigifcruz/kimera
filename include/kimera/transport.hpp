#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "kimera/kimera.hpp"
#include <cstddef>

#define MAX_KEY_LEN     256
#define DEFAULT_KEY_LEN 64

#ifdef KIMERA_WINDOWS
#include <ws2tcpip.h>
#include <afunix.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include <sys/types.h>
#include <stdbool.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

typedef struct sockaddr_in socket_in;
typedef struct sockaddr_un socket_un;
typedef struct sockaddr socket_t;

typedef struct {
    uint64_t pts;
    uint32_t len;
    uint32_t i;
    uint32_t n;
    char* payload;
} Packet;

class Router {
public:
    Router(Kimera*);
    ~Router();

    bool MakePacket(AVPacket*);
    bool ParsePacket();

    char* GetBuffer();
    AVPacket* GetPacket();

    int GetHeaderSize();
    int GetPacketSize();

private:
    int payload_size;
    int header_size;
    int packet_size;
    uint32_t checksum;
    Packet* packet = NULL;
    char* buffer = NULL;

    size_t GetSize(Packet*, size_t);

    // Utils
    static inline uint32_t
    buffer_read32be(const uint8_t *buf) {
        return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
    }

    static inline uint64_t
    buffer_read64be(const uint8_t *buf) {
        uint32_t msb = buffer_read32be(buf);
        uint32_t lsb = buffer_read32be(&buf[4]);
        return ((uint64_t) lsb << 32) | msb;
    }
};

class Crypto {
public:
    Crypto(Kimera*);
    ~Crypto();

    static bool NewKey(char*, size_t);
    static int Bytes2Base(char*, size_t, char*);
    static int Base2Bytes(char*, size_t, char*);

    bool Connect(unsigned int);
    bool Accept(unsigned int);

    int Send(const void*, size_t);
    int Recv(void*, size_t);

private:
    const SSL_METHOD *method = NULL;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;

    const char* GetCypher();
};

class Socket {
public:
    Socket(Kimera*);
    ~Socket();

    bool OpenServer();
    bool OpenClient();

    void SendPacket(AVPacket*);
    AVPacket* RecvPacket();

    int SendBuffer(const void*, size_t);
    int RecvBuffer(void*, size_t);

private:
    int server_fd;
    int client_fd;
    Interfaces interf;
    Kimera* state = NULL;

    // Crypto
    Crypto crypto;

    // Router
    Router router;

    // Socket Descriptors
    socket_in* client_in;
    socket_in* server_in;
    socket_un* client_un;
    socket_un* server_un;

    // TCP Methods
    bool OpenTCPClient();
    bool OpenTCPServer();
    void CloseTCP();

    int SendTCP(const void*, size_t);
    int RecvTCP(void*, size_t);

    // UDP Methods
    bool OpenUDPClient();
    bool OpenUDPServer();
    void CloseUDP();

    int SendUDP(const void*, size_t);
    int RecvUDP(void*, size_t);

    // UNIX Mehtods
    bool OpenUNIXClient();
    bool OpenUNIXServer();
    void CloseUNIX();

    int SendUNIX(const void*, size_t);
    int RecvUNIX(void*, size_t);
};

#endif
