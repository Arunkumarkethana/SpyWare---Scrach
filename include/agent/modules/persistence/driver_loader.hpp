#pragma once
#include <windows.h>
#include <string>

class DriverLoader {
public:
    static bool LoadDriver(const std::string& driverPath, const std::string& serviceName);
    static bool UnloadDriver(const std::string& serviceName);
    static bool HidePID(ULONG pid);
};
