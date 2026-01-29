#pragma once
#include <vector>
#include <string>
#include <algorithm>

class RC4 {
private:
    unsigned char S[256];
    int i = 0;
    int j = 0;

public:
    RC4(const std::vector<unsigned char>& key) {
        // KSA (Key-Scheduling Algorithm)
        for (int k = 0; k < 256; k++) S[k] = k;

        int j_init = 0;
        for (int k = 0; k < 256; k++) {
            j_init = (j_init + S[k] + key[k % key.size()]) % 256;
            std::swap(S[k], S[j_init]);
        }
    }

    // PRGA (Pseudo-Random Generation Algorithm)
    // Modifies stream in-place or returns new buffer
    void Cipher(std::vector<unsigned char>& data) {
        for (size_t k = 0; k < data.size(); k++) {
            i = (i + 1) % 256;
            j = (j + S[i]) % 256;
            std::swap(S[i], S[j]);
            unsigned char K = S[(S[i] + S[j]) % 256];
            data[k] ^= K;
        }
    }
    
    // String wrapper
    std::string Encrypt(const std::string& input) {
        std::vector<unsigned char> buf(input.begin(), input.end());
        Cipher(buf);
        return std::string(buf.begin(), buf.end());
    }
};
