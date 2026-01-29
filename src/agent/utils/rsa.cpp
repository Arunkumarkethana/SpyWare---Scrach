#include "agent/utils/rsa.hpp"
#include <iostream>

#pragma comment(lib, "advapi32.lib")

RSA::RSA() : hProv(0), hKey(0) {
    if (!CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        // Handle error
    }
}

RSA::~RSA() {
    if (hKey) CryptDestroyKey(hKey);
    if (hProv) CryptReleaseContext(hProv, 0);
}

bool RSA::ImportPublicKey(const std::vector<unsigned char>& keyBlob) {
    if (!hProv) return false;
    
    if (hKey) {
        CryptDestroyKey(hKey);
        hKey = 0;
    }
    
    return CryptImportKey(hProv, keyBlob.data(), keyBlob.size(), 0, 0, &hKey);
}

std::vector<unsigned char> RSA::Encrypt(const std::vector<unsigned char>& data) {
    if (!hKey) return {};

    std::vector<unsigned char> buffer = data;
    DWORD dataLen = (DWORD)data.size();
    DWORD bufLen = dataLen; // Check required size?
    
    // For RSA, block size matters. CryptoAPI handles padding (PKCS1 usually).
    // We need to determine the buffer size. 
    // Usually key size (e.g. 2048 bits / 8 = 256 bytes).
    DWORD keyLen = 0;
    DWORD d = sizeof(DWORD);
    CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE*)&keyLen, &d, 0);
    keyLen /= 8; 
    
    // Safe buffer size
    if (bufLen < keyLen) {
        buffer.resize(keyLen);
    }
    
    if (!CryptEncrypt(hKey, 0, TRUE, 0, buffer.data(), &dataLen, buffer.size())) {
        // Use GetLastError() to debug if needed
        return {};
    }
    
    buffer.resize(dataLen);
    return buffer;
}

bool RSA::VerifySignature(const std::vector<unsigned char>& data, const std::vector<unsigned char>& signature) {
    if (!hKey) return false;

    HCRYPTHASH hHash = 0;
    // Create Hash Object (SHA-256 is ideal, ensuring provider supports it. Fallback to SHA1 if needed)
    // CALG_SHA_256 might require MS_ENH_RSA_AES_PROV. 
    // Checking if CALG_SHA1 works for now to ensure compatibility with PROV_RSA_FULL.
    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
        return false;
    }

    // Hash Data
    if (!CryptHashData(hHash, data.data(), data.size(), 0)) {
        CryptDestroyHash(hHash);
        return false;
    }

    // Verify Signature
    // Note: signature is expected to be Little-Endian for CryptoAPI usually? 
    // Or standard Raw. 
    BOOL result = CryptVerifySignature(hHash, signature.data(), signature.size(), hKey, NULL, 0);
    
    CryptDestroyHash(hHash);
    return result;
}
