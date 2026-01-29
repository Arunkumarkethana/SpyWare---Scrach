#include "exfil/tcp_client.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

TcpClient::TcpClient(const std::string& ipAddress, int port) : ip(ipAddress), port(port) {
    sock = INVALID_SOCKET;
    isConnected = false;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

TcpClient::~TcpClient() {
    if(sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();
}

bool TcpClient::Connect() {
    if(sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if(getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) return false;

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        freeaddrinfo(res);
        return false;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        freeaddrinfo(res);
        return false;
    }

    freeaddrinfo(res);
    isConnected = true;
    return true;
}

bool TcpClient::SendData(const std::string& data) {
    // 1. If not connected, try to connect
    if (!isConnected) {
        if (!Connect()) return false;
    }

    // 2. Try Sending
    std::string packet = data + "\n";
    int sent = send(sock, packet.c_str(), packet.length(), 0);

    // 3. Handle Failure (Broken Pipe usually)
    if (sent == SOCKET_ERROR) {
        isConnected = false;
        
        // Retry ONCE
        if(Connect()) {
            send(sock, packet.c_str(), packet.length(), 0);
            return true;
        }
        return false;
    }

    return true;
}
