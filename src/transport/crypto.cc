#include "kimera/transport.hpp"

const Kimera* crypto_state;

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

    return Crypto::Base2Bytes(crypto_state->psk_key, b64_len, (char*)psk);
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

    return Crypto::Base2Bytes(crypto_state->psk_key, b64_len, (char*)psk);
}

Crypto::Crypto(Kimera* state) {
    if (!(state->pipe & CRYPTO)) return;

    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    if (state->packet_size >= 31000) {
        printf("[CRYPTO] Invalid settings, packet_size must be smaller than 31 KB.\n");
        throw "error";
    }

    method = TLS_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        printf("[CRYPTO] Unable to create SSL context.\n");
        ERR_print_errors_fp(stderr);
        throw "error";
    }

    crypto_state = state;
}

Crypto::~Crypto() {
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (ctx)
        SSL_CTX_free(ctx);
    EVP_cleanup();
}

bool Crypto::NewKey(char* bin, size_t bin_len) {
    if (RAND_bytes((unsigned char*)bin, bin_len) != 1) {
        printf("[CRYPTO] Can't generate random key.\n");
        return false;
    }
    return true;
}

int Crypto::Bytes2Base(char* bin, size_t bin_len, char* b64) {
    if (!bin || !b64) return -1;
    return EVP_EncodeBlock((unsigned char*)b64, (const unsigned char*)bin, bin_len);
}

int Crypto::Base2Bytes(char* b64, size_t b64_len, char* bin) {
    if (!bin || !b64) return -1;
    return EVP_DecodeBlock((unsigned char*)bin, (const unsigned char*)b64, b64_len);
}

const char* Crypto::GetCypher() {
    return SSL_CIPHER_get_name(SSL_get_current_cipher(ssl));
}

bool Crypto::Accept(unsigned int fd) {
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    SSL_set_psk_server_callback(ssl, psk_server_cb);
    if (SSL_accept(ssl) <= 0) {
        printf("[CRYPTO] Couldn't accept secure connection.\n");
        ERR_print_errors_fp(stderr);
        return false;
    }
    printf("[CRYPTO] Negotiated cipher set: %s\n", GetCypher());
    return true;
}

bool Crypto::Connect(unsigned int fd) {
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    SSL_set_psk_client_callback(ssl, psk_client_cb);
    if (SSL_connect(ssl) <= 0) {
        printf("[CRYPTO] Couldn't accept secure connection.\n");
        ERR_print_errors_fp(stderr);
        return false;
    }
    printf("[CRYPTO] Negotiated cipher set: %s\n", GetCypher());
    return true;
}

int Crypto::Send(const void* buf, size_t len) {
    return SSL_write(ssl, (void*)buf, len);
}

int Crypto::Recv(void* buf, size_t len) {
    return SSL_read(ssl, buf, len);
}
