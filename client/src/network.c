#include "network.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

SOCKET sock = INVALID_SOCKET;
int connected = 0;

int connect_to_server() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        return 0;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return 0;
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    // Timeout de conexão (5 segundos)
    struct timeval timeout = {5, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        closesocket(sock);
        return 0;
    }

    connected = 1;
    return 1;
}

void send_encrypted_data(const char* data) {
    if (!connected && !connect_to_server()) {
        return;
    }

    char encrypted[512];
    encrypt_data(data, encrypted, sizeof(encrypted));
    
    // Enviar tamanho primeiro
    int size = (int)strlen(encrypted);
    if (send(sock, (char*)&size, sizeof(int), 0) == SOCKET_ERROR) {
        connected = 0;
        return;
    }
    
    // Enviar dados criptografados
    if (send(sock, encrypted, size, 0) == SOCKET_ERROR) {
        connected = 0;
    }
}

void cleanup_network() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();
}
