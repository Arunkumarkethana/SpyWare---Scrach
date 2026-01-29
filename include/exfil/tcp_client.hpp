#pragma once
#include <string>
#include <winsock2.h>

class TcpClient {
private:
    std::string ip;
    int port;
    SOCKET sock;
    bool isConnected;
    
    bool Connect(); // Internal helper

public:
    TcpClient(const std::string& ipAddress, int port);
    ~TcpClient(); // Destructor to close
    
    // Disable Copying (RAII Safety)
    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    bool SendData(const std::string& data);
};
