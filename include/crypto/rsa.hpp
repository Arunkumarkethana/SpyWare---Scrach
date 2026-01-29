#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include <wincrypt.h>

class RSA {
private:
    HCRYPTPROV hProv;
    HCRYPTKEY hKey;

public:
    RSA();
    ~RSA();
    
    // Import Public Key Blob (for Agent)
    bool ImportPublicKey(const std::vector<unsigned char>& keyBlob);
    
    // Encrypt Data (Public Key)
    std::vector<unsigned char> Encrypt(const std::vector<unsigned char>& data);
    
    // Verify Signature (Public Key)
    bool VerifySignature(const std::vector<unsigned char>& data, const std::vector<unsigned char>& signature);
};
