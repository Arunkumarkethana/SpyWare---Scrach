#pragma once
#include <windows.h>
#include <winternl.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <intrin.h>

#pragma pack(push, 1)
struct SyscallFrame {
    uint8_t mov_r10_rcx[3];
    uint8_t mov_eax;
    uint32_t syscall_id;    
    uint8_t syscall_inst[2];
    uint8_t ret;
};
#pragma pack(pop)

typedef struct _MY_LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} MY_LDR_DATA_TABLE_ENTRY, *PMY_LDR_DATA_TABLE_ENTRY;

class DirectSyscall {
private:
    HMODULE GetNtdllBase() {
#ifdef _WIN64
        PPEB peb = (PPEB)__readgsqword(0x60);
#else
        PPEB peb = (PPEB)__readfsdword(0x30);
#endif
        PLIST_ENTRY head = &peb->Ldr->InMemoryOrderModuleList;
        PLIST_ENTRY entry = head->Flink;
        while(entry != head) {
            PMY_LDR_DATA_TABLE_ENTRY module = CONTAINING_RECORD(entry, MY_LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
            if(module->BaseDllName.Buffer) {
                 if ((module->BaseDllName.Buffer[0] == L'n' || module->BaseDllName.Buffer[0] == L'N') &&
                    (module->BaseDllName.Buffer[1] == L't' || module->BaseDllName.Buffer[1] == L'T')) {
                    return (HMODULE)module->DllBase;
                }
            }
            entry = entry->Flink;
        }
        return nullptr;
    }

    uint32_t FindSyscallId(const char* funcName) {
        HMODULE ntdll = GetNtdllBase();
        if (!ntdll) return 0;
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)ntdll;
        PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((uintptr_t)ntdll + dos->e_lfanew);
        PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((uintptr_t)ntdll + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        uint32_t* names = (uint32_t*)((uintptr_t)ntdll + exports->AddressOfNames);
        uint32_t* funcs = (uint32_t*)((uintptr_t)ntdll + exports->AddressOfFunctions);
        uint16_t* ords = (uint16_t*)((uintptr_t)ntdll + exports->AddressOfNameOrdinals);
        for(uint32_t i = 0; i < exports->NumberOfNames; i++) {
            char* name = (char*)((uintptr_t)ntdll + names[i]);
            if(strcmp(name, funcName) == 0) {
                uintptr_t funcAddr = (uintptr_t)ntdll + funcs[ords[i]];
                for(uintptr_t j = funcAddr; j < funcAddr + 0x100; j++) {
                    if(*(uint8_t*)j == 0x4c && *(uint8_t*)(j+1) == 0x8b && *(uint8_t*)(j+2) == 0xd1 && *(uint8_t*)(j+3) == 0xb8) {
                        return *(uint32_t*)(j + 4);
                    }
                }
            }
        }
        return 0;
    }

public:
    template<typename... Args>
    NTSTATUS Execute(const char* funcName, Args... args) {
        uint32_t syscallId = FindSyscallId(funcName);
        if(!syscallId) return (NTSTATUS)0xC000007A;
        SyscallFrame frame;
        frame.mov_r10_rcx[0] = 0x4c; frame.mov_r10_rcx[1] = 0x8b; frame.mov_r10_rcx[2] = 0xd1;
        frame.mov_eax = 0xb8;
        frame.syscall_id = syscallId;
        frame.syscall_inst[0] = 0x0f; frame.syscall_inst[1] = 0x05;
        frame.ret = 0xc3;
        DWORD oldProtect;
        VirtualProtect(&frame, sizeof(frame), PAGE_EXECUTE_READWRITE, &oldProtect);
        using SyscallFunc = NTSTATUS(__fastcall*)(Args...);
        auto fn = (SyscallFunc)&frame;
        NTSTATUS status = fn(args...);
        VirtualProtect(&frame, sizeof(frame), oldProtect, &oldProtect);
        return status;
    }
    
    // Stub for main.cpp
    void HookNtUserGetAsyncKeyState() {}
};
