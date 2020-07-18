#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "kimera/state.h"

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

typedef struct {
    int payload_size;
    int header_size;
    int packet_size;
    uint32_t checksum;
    Packet* packet;
    char* buffer;
} RouterState;

typedef struct {
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL *ssl;
} CryptoState;

typedef struct {
    int server_fd;
    int client_fd;
    Interfaces interf;

    // Crypto
    CryptoState* crypto;

    // Router
    RouterState* router;
    Packet* packet;

    // UDP & TCP Sockets
    socket_in* client_in;
    socket_in* server_in;

    // UNIX Sockets
    socket_un* client_un;
    socket_un* server_un;
} SocketState;

//
// General Methods
//

SocketState* init_socket();
void close_socket(SocketState*);

bool open_socket_server(SocketState*, State*);
bool open_socket_client(SocketState*, State*);

int socket_recv_packet(SocketState*);
void socket_send_packet(SocketState*, AVPacket*);

int socket_send_buffer(SocketState*, const void*, size_t);
int socket_recv_buffer(SocketState*, void*, size_t);

//
// Router Methods
//

RouterState* init_router();
void close_router(RouterState*);

bool start_router(RouterState*, State*);

bool router_make_packet(RouterState*, AVPacket*);
bool router_parse_packet(RouterState*);

//
// TCP Socket Methods
//

bool open_tcp_client(SocketState*, State*);
bool open_tcp_server(SocketState*, State*);
void close_tcp(SocketState*);
int send_tcp(SocketState*, const void*, size_t);
int recv_tcp(SocketState*, void*, size_t);

//
// UDP Socket Methods
//

bool open_udp_client(SocketState*, State*);
bool open_udp_server(SocketState*, State*);
void close_udp(SocketState*);
int send_udp(SocketState*, const void*, size_t);
int recv_udp(SocketState*, void*, size_t);

//
// UNIX Socket Methods
//

bool open_unix_client(SocketState*, State*);
bool open_unix_server(SocketState*, State*);
void close_unix(SocketState*);
int send_unix(SocketState*, const void*, size_t);
int recv_unix(SocketState*, void*, size_t);

//
// Crypto Methods
//

CryptoState* init_crypto();
void close_crypto();

bool crypto_new_key(char*, size_t);
int crypto_bytes_to_b64(char*, size_t, char*);
int crypto_b64_to_bytes(char*, size_t, char*);

bool start_crypto(CryptoState*, State*);
bool crypto_connect(CryptoState*, unsigned int);
bool crypto_accept(CryptoState*, unsigned int);

int crypto_send(SocketState*, const void*, size_t);
int crypto_recv(SocketState*, void*, size_t);

//
// Util Methods
//

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

#endif
