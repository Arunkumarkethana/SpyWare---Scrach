#include <string>
#include <winsock2.h>
#include <vector>
#include <mutex>

class TcpClient {
private:
    struct Endpoint {
        std::string ip;
        int port;
    };
    std::vector<Endpoint> endpoints;
    std::string frontDomain;
    size_t currentEndpointIndex;
    SOCKET sock;
    bool isConnected;
    
    bool Connect(); // Internal helper

public:
    TcpClient(const std::string& defaultIp, int defaultPort);
    ~TcpClient(); // Destructor to close
    
    // Disable Copying (RAII Safety)
    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    bool SendData(const std::string& data);
    void AddEndpoint(const std::string& ip, int port);
    void SetFrontDomain(const std::string& domain);
    void SetTarget(const std::string& newIp, int newPort); // For backward compatibility

private:
    class RC4* encryption; // Forward declared, proper type
    std::vector<unsigned char> key;
    std::mutex netMtx;
};
