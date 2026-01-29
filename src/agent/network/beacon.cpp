#include <winsock2.h>
#include "agent/network/beacon.hpp"
#include "agent/utils/rsa.hpp"
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

void Beacon::Shout() {
    // 1. Prepare Payload
    std::string payload = "AGENT_ALIVE"; 
    // In real scenario: "Hostname|IP|OS" using SystemInfo
    
    // 2. Encrypt (Asymmetric)
    RSA rsa;
    // TODO: Import Real Public Key Blob here.
    // std::vector<unsigned char> pubKey = { ... };
    // rsa.ImportPublicKey(pubKey);
    // For prototype, we verify compilation.
    
    std::vector<unsigned char> data(payload.begin(), payload.end());
    std::vector<unsigned char> encrypted = rsa.Encrypt(data);
    
    if(encrypted.empty()) return; // Encryption failed (no key)

    // 3. Broadcast UDP
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) return;

    BOOL broadcast = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast));

    struct sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(9999);
    recvAddr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(sock, (const char*)encrypted.data(), encrypted.size(), 0, (sockaddr*)&recvAddr, sizeof(recvAddr));

    closesocket(sock);
}
