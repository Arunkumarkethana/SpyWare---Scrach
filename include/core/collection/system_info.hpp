#pragma once
#include <string>
#include <vector>

class SystemInfo {
public:
    static std::string GetUsername();
    static std::string GetHostName();
    static std::string GetOSVersion();
    static std::string GetIPAddress();
    
    static std::string GetAllInfo();
};
