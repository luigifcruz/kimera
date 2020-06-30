#include "kimera/transport.h"

bool open_crypto(State* state, CryptoState* crypto, bool is_server) {
    crypto = (CryptoState*)malloc(sizeof(CryptoState));

    SSL_library_init();
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();

    if (is_server)
        crypto->method = SSLv23_server_method();
    else
        crypto->method = SSLv23_client_method();
    
    crypto->ctx = SSL_CTX_new(crypto->method);
    if (!crypto->ctx) {
        printf("[TCP_SSL_SOCKET] Unable to create SSL context.\n");
        ERR_print_errors_fp(stderr);
        return false;
    }

    SSL_CTX_set_ecdh_auto(crypto->ctx, 1);

    if (SSL_CTX_use_certificate_file(crypto->ctx, state->ssl_cert, SSL_FILETYPE_PEM) <= 0) {
        printf("[TCP_SSL_SOCKET] Couldn't open SSL certificate.\n");
        ERR_print_errors_fp(stderr);
	    return false;
    }

    if (SSL_CTX_use_PrivateKey_file(crypto->ctx, state->ssl_key, SSL_FILETYPE_PEM) <= 0) {
        printf("[TCP_SSL_SOCKET] Couldn't open SSL key.");
        ERR_print_errors_fp(stderr);
	    return false;
    }

    if (!SSL_CTX_check_private_key(crypto->ctx)) {
        printf("[TCP_SSL_SOCKET] Couldn't validate public certificate.\n");
        return false;
    }

    crypto->ssl = SSL_new(crypto->ctx);

    return true;
}

void close_cryto(CryptoState* crypto) {
    SSL_shutdown(crypto->ssl);
    SSL_free(crypto->ssl);
    SSL_CTX_free(crypto->ctx);
    EVP_cleanup();
}

bool open_tcp_ssl_client(SocketState* sock_state, State* state) {
    if (!open_crypto(state, sock_state->crypto, false))
        return false;

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

    printf("SSL_connect 1\n");
    SSL_set_fd(sock_state->crypto->ssl, sock_state->server_fd);
    printf("SSL_connect 1.5\n");
    if (SSL_connect(sock_state->crypto->ssl) <= 0) {
        printf("[TCP_SSL_SOCKET] Couldn't accept secure connection.\n");
        ERR_print_errors_fp(stderr);
        return false;
    }
    printf("SSL_connect 2\n");

    sock_state->interf = TCP_SSL;
    return true;
}

bool open_tcp_ssl_server(SocketState* sock_state, State* state) {
    if (!open_crypto(state, sock_state->crypto, true))
        return false;

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
    
    printf("SSL_accept 1\n");
    SSL_set_fd(sock_state->crypto->ssl, sock_state->client_fd);
    printf("SSL_accept 1.5\n");

    if (SSL_accept(sock_state->crypto->ssl) <= 0) {
        printf("[TCP_SSL_SOCKET] Couldn't accept secure connection.\n");
        ERR_print_errors_fp(stderr);
        return false;
    }
    printf("SSL_accept 2\n");

    printf("[TCP_SSL_SOCKET] Client connected.\n");

    sock_state->interf = TCP_SSL;
    return true;
}

void close_tcp_ssl(SocketState* sock_state) {
    close_cryto(sock_state->crypto);

    close(sock_state->client_fd);
    close(sock_state->server_fd);

    sock_state->interf = NONE;
    sock_state->crypto = NULL;
}

int send_tcp_ssl(SocketState* socket, const void* buf, size_t len) {
    //return SSL_write(socket->crypto->ssl, (void*)buf, len);
    return len;
}

int recv_tcp_ssl(SocketState* socket, void* buf, size_t len) {
    //return SSL_read(socket->crypto->ssl, buf, len);
    return len;
}