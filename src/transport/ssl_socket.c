#include "kimera/transport.h"

const State* crypto_state;

CryptoState* init_crypto() {
    CryptoState* crypto = (CryptoState*)malloc(sizeof(CryptoState));
    crypto->ctx    = NULL;
    crypto->ssl    = NULL;
    crypto->method = NULL;
    crypto_state   = NULL;
    return crypto;
}

void close_crypto(CryptoState* crypto) {
    if (crypto->ssl) {
        SSL_shutdown(crypto->ssl);
        SSL_free(crypto->ssl);
    }
    if (crypto->ctx)
        SSL_CTX_free(crypto->ctx);
    EVP_cleanup();
}

char* crypto_new_key(size_t len) {
    char* buffer = (char*)malloc(len);
    if (RAND_bytes((unsigned char*)buffer, len) != 1) {
        printf("[CRYPTO] Can't generate random key.\n");
        return NULL;
    }
    return buffer;
}

char* crypto_bytes_to_b64(char* key, size_t len) {
    if (!key) return NULL;
    char* b64 = (char*)malloc(MAX_KEY_LEN);
    EVP_EncodeBlock((unsigned char*)b64, (const unsigned char*)key, len);
    return b64;
}

char* crypto_b64_to_bytes(char* b64) {
    if (!b64) return NULL;
    size_t b64_len = strlen(b64);
    char* key = (char*)malloc(MAX_KEY_LEN);
    EVP_DecodeBlock((unsigned char*)key, (const unsigned char*)b64, b64_len);
    return key;
}

const char* crypto_get_cipher(CryptoState* crypto) {
    return SSL_CIPHER_get_name(SSL_get_current_cipher(crypto->ssl));
}

bool start_crypto(CryptoState* crypto, State* state) {
    SSL_load_error_strings();	
    OpenSSL_add_all_algorithms();

    if (state->packet_size >= 31000) {
        printf("[CRYPTO] Invalid settings, packet_size must be smaller than 31 KB.\n");
        return false;
    }

    if (state->mode & TRANSMITTER && strlen(state->psk_key) < 8) {
        printf("\nNo pre-shared key found, generating one...\n");
        printf("TIP: This can be pre-defined in the configuration file as `psk_key`.\n\n");
        
        state->psk_key = crypto_new_key(DEFAULT_KEY_LEN);
        char* b64_key = crypto_bytes_to_b64(state->psk_key, DEFAULT_KEY_LEN);
        printf("%s\n\n", b64_key);
        free(b64_key);
    }

    if (state->mode & RECEIVER && strlen(state->psk_key) < 8) {
        printf("\nNo pre-shared key found! Type the Transmitter Pre-shared Key:\n");
        printf("TIP: This can be pre-defined in the configuration file as `psk_key`.\n\n");
        
        char* b64_key = (char*)malloc(MAX_KEY_LEN);
        printf("KEY> ");
        int values = scanf("%s", b64_key);
        printf("\n");

        if (values == 1 && !(state->psk_key = crypto_b64_to_bytes(b64_key))) {
            printf("[CRYPTO] User entered an invalid pre-shared key. Exiting...\n");
            return false;
        }

        free(b64_key);
    }

    crypto->method = TLS_method();
    crypto->ctx = SSL_CTX_new(crypto->method);
    if (!crypto->ctx) {
        printf("[CRYPTO] Unable to create SSL context.\n");
        ERR_print_errors_fp(stderr);
        return false;
    }

    crypto_state = state;
    return true;
}

static unsigned int psk_client_cb(SSL *ssl, const char *hint, char *id, unsigned int max_id_len, unsigned char *psk, unsigned int max_psk_len) {
    (void)(ssl);
    (void)(hint);

    if (!crypto_state) {
        printf("[CRYPTO] Global state not loaded.\n");
        return 0;
    }

    size_t id_len = strlen(crypto_state->psk_identity);
    if (id_len >= max_id_len) {
        printf("[CRYPTO] The PSK identity is too long.\n");
        return 0;
    }

    size_t psk_len = strlen(crypto_state->psk_key);
    if (psk_len >= max_psk_len) {
        printf("[CRYPTO] Pre-shared key is too long. Max length is %d and have %ld.\n", max_psk_len, psk_len);
        return 0;
    }

    strcpy(id, crypto_state->psk_identity);
    memcpy(psk, crypto_state->psk_key, psk_len);
    return psk_len;
}

static unsigned int psk_server_cb(SSL* ssl, const char* id, unsigned char* psk, unsigned int max_len) {
    (void)(ssl);

    if (!id) {
        printf("[CRYPTO] The PSK identity wasn't found.\n");
        return 0;
    }

    if (!crypto_state) {
        printf("[CRYPTO] Global state not loaded.\n");
        return 0;
    }

    if (strcmp(id, crypto_state->psk_identity) != 0) {
        printf("[CRYPTO] Can't validate PSK identity.\n");
        return 0;
    }

    size_t psk_len = strlen(crypto_state->psk_key);
    if (psk_len >= max_len) {
        printf("[CRYPTO] Pre-shared key is too long. Max length is %d and have %ld.\n", max_len, psk_len);
        return 0;
    }

    memcpy(psk, crypto_state->psk_key, psk_len);
    return psk_len;
}

bool crypto_accept(CryptoState* crypto, unsigned int fd) {
    crypto->ssl = SSL_new(crypto->ctx);
    SSL_set_fd(crypto->ssl, fd);
    SSL_set_psk_server_callback(crypto->ssl, psk_server_cb);
    if (SSL_accept(crypto->ssl) <= 0) {
        printf("[CRYPTO] Couldn't accept secure connection.\n");
        ERR_print_errors_fp(stderr);
        return false;
    }
    printf("[CRYPTO] Negotiated cipher set: %s\n", crypto_get_cipher(crypto));
    return true;
}

bool crypto_connect(CryptoState* crypto, unsigned int fd) {
    crypto->ssl = SSL_new(crypto->ctx);
    SSL_set_fd(crypto->ssl, fd);
    SSL_set_psk_client_callback(crypto->ssl, psk_client_cb);
    if (SSL_connect(crypto->ssl) <= 0) {
        printf("[CRYPTO] Couldn't accept secure connection.\n");
        ERR_print_errors_fp(stderr);
        return false;
    }
    printf("[CRYPTO] Negotiated cipher set: %s\n", crypto_get_cipher(crypto));
    return true;
}

bool open_tcp_ssl_client(SocketState* sock_state, State* state) {
    sock_state->server_in = (socket_in*)malloc(sizeof(socket_in));

    if ((sock_state->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SSL_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    memset(sock_state->server_in, 0, sizeof(socket_in)); 

    sock_state->server_in->sin_family = AF_INET;
    sock_state->server_in->sin_port = htons(state->port);

    if (inet_pton(AF_INET, state->address, &sock_state->server_in->sin_addr) < 0) { 
        printf("[TCP_SSL_SOCKET] Invalid address.\n"); 
        return false;
    } 

    if (connect(sock_state->server_fd, (socket_t*)sock_state->server_in, sizeof(socket_in)) < 0) {
        printf("[TCP_SSL_SOCKET] Couldn't connect to server.\n");
        return false;
    }

    sock_state->crypto = init_crypto();
    if (!start_crypto(sock_state->crypto, state))
        return false;

    if (!crypto_connect(sock_state->crypto, sock_state->server_fd))
        return false;

    printf("[TCP_SSL_SOCKET] Connected to server...\n");

    sock_state->interf = TCP_SSL;
    return true;
}

bool open_tcp_ssl_server(SocketState* sock_state, State* state) {
    sock_state->server_in = (socket_in*)malloc(sizeof(socket_in));
    sock_state->client_in = (socket_in*)malloc(sizeof(socket_in));

    if ((sock_state->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[TCP_SSL_SOCKET] Couldn't open stream socket.\n");
        return false;
    }

    if (setsockopt(sock_state->server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        printf("[TCP_SSL_SOCKET] Failed trying to reuse socket.\n");
    }
    
    memset(sock_state->server_in, 0, sizeof(socket_in)); 
    memset(sock_state->client_in, 0, sizeof(socket_in)); 

    sock_state->server_in->sin_family = AF_INET; 
    sock_state->server_in->sin_addr.s_addr = htonl(INADDR_ANY); 
    sock_state->server_in->sin_port = htons(state->port); 

    if (bind(sock_state->server_fd, (socket_t*)sock_state->server_in, sizeof(socket_in)) < 0) {
        printf("[TCP_SSL_SOCKET] Couldn't open server port.\n");
        return false;
    }

    if ((listen(sock_state->server_fd, 1)) != 0) { 
        printf("[TCP_SSL_SOCKET] Failed to listen.\n");
        return false;
    }

    printf("[TCP_SSL_SOCKET] Waiting client.\n");

    unsigned int len = sizeof(socket_in); 
    if ((sock_state->client_fd = accept(sock_state->server_fd, (socket_t*)sock_state->client_in, &len)) < 0) {
        printf("[TCP_SSL_SOCKET] Couldn't accept the client.\n");
        return false;
    }

    sock_state->crypto = init_crypto();
    if (!start_crypto(sock_state->crypto, state))
        return false;

    if (!crypto_accept(sock_state->crypto, sock_state->client_fd))
        return false;

    printf("[TCP_SSL_SOCKET] Client connected...\n");

    sock_state->interf = TCP_SSL;
    return true;
}

void close_tcp_ssl(SocketState* sock_state) {
    if (sock_state->crypto)
        close_crypto(sock_state->crypto);

    close(sock_state->client_fd);
    close(sock_state->server_fd);

    sock_state->interf = NONE;
    sock_state->crypto = NULL;
}

int send_tcp_ssl(SocketState* socket, const void* buf, size_t len) {
    return SSL_write(socket->crypto->ssl, (void*)buf, len);
}

int recv_tcp_ssl(SocketState* socket, void* buf, size_t len) {
    return SSL_read(socket->crypto->ssl, buf, len);
}