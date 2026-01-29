#include "core/collection/system_info.hpp"
#include <windows.h>
#include <iphlpapi.h>
#include <sstream>

#pragma comment(lib, "iphlpapi.lib")

std::string SystemInfo::GetUsername() {
    char buf[256];
    DWORD len = sizeof(buf);
    if(GetUserNameA(buf, &len)) return std::string(buf);
    return "Unknown";
}

std::string SystemInfo::GetHostName() {
    char buf[256];
    DWORD len = sizeof(buf);
    if(GetComputerNameA(buf, &len)) return std::string(buf);
    return "Unknown";
}

std::string SystemInfo::GetOSVersion() {
    // GetVersionEx is deprecated but works for basic checks or needs manifest
    // For simplicity, we stick to basic API or registry read
    return "Windows"; // Placeholder for complexity reduction
}

std::string SystemInfo::GetIPAddress() {
    IP_ADAPTER_INFO AdapterInfo[16];
    DWORD dwBufLen = sizeof(AdapterInfo);
    
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
    if(dwStatus == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        while (pAdapterInfo) {
            std::string ip = pAdapterInfo->IpAddressList.IpAddress.String;
            if(ip != "0.0.0.0" && ip != "127.0.0.1") return ip;
            pAdapterInfo = pAdapterInfo->Next;
        }
    }
    return "0.0.0.0";
}

std::string SystemInfo::GetAllInfo() {
    std::stringstream ss;
    ss << "User: " << GetUsername() << " | ";
    ss << "Host: " << GetHostName() << " | ";
    ss << "OS: " << GetOSVersion() << " | ";
    ss << "IP: " << GetIPAddress();
    return ss.str();
}
