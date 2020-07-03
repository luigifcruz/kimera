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

bool crypto_new_key(char* bin, size_t bin_len) {
    if (RAND_bytes((unsigned char*)bin, bin_len) != 1) {
        printf("[CRYPTO] Can't generate random key.\n");
        return false;
    }
    return true;
}

int crypto_bytes_to_b64(char* bin, size_t bin_len, char* b64) {
    if (!bin || !b64) return -1;
    return EVP_EncodeBlock((unsigned char*)b64, (const unsigned char*)bin, bin_len);
}

int crypto_b64_to_bytes(char* b64, size_t b64_len, char* bin) {
    if (!bin || !b64) return -1;
    return EVP_DecodeBlock((unsigned char*)bin, (const unsigned char*)b64, b64_len);
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

    if (strlen(state->psk_key) < 8) {
        if (state->mode & TRANSMITTER) {
            char bin_key[DEFAULT_KEY_LEN];
            if (!crypto_new_key(bin_key, DEFAULT_KEY_LEN)) return false;
            crypto_bytes_to_b64((char*)&bin_key, DEFAULT_KEY_LEN, state->psk_key);

            printf("\nNo pre-shared key found, generating one...\n");
            printf("TIP: This can be pre-defined in the configuration file as `psk_key`.\n\n");
            printf("%s\n\n", state->psk_key);
        }

        if (state->mode & RECEIVER) {
            printf("\nNo pre-shared key found! Type the Transmitter Pre-shared Key:\n");
            printf("TIP: This can be pre-defined in the configuration file as `psk_key`.\n\n");
            printf("KEY> ");

            if (scanf("%s",  state->psk_key) != 1) {
                printf("[CRYPTO] User entered an invalid pre-shared key. Exiting...\n");
                return false;
            }

            printf("\n");
        }
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

    size_t id_len = strlen(KIMERA_PSK_IDENTITY);
    if (id_len >= max_id_len) {
        printf("[CRYPTO] The PSK identity is too long.\n");
        return 0;
    }

    size_t b64_len = strlen(crypto_state->psk_key);
    if ((size_t)(((float)b64_len/0.75)+0.5) >= max_psk_len) {
        printf("[CRYPTO] Pre-shared key is too long.");
        return 0;
    }

    strcpy(id, KIMERA_PSK_IDENTITY);

    return crypto_b64_to_bytes(crypto_state->psk_key, b64_len, (char*)psk);
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

    if (strcmp(id, KIMERA_PSK_IDENTITY) != 0) {
        printf("[CRYPTO] Can't validate PSK identity.\n");
        return 0;
    }

    size_t b64_len = strlen(crypto_state->psk_key);
    if ((size_t)(((float)b64_len*0.75)+0.5) >= max_len) {
        printf("[CRYPTO] Pre-shared key is too long.");
        return 0;
    }

    return crypto_b64_to_bytes(crypto_state->psk_key, b64_len, (char*)psk);
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

    sock_state->interf = TCP;//TCP_SSL;
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

    sock_state->interf = TCP;
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
