#pragma once
#include <windows.h>
#include <winternl.h>

#define DLLEXPORT __declspec(dllexport)

// MinGW compatibility definitions
typedef struct _PEB_LDR_DATA_CUSTOM {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA_CUSTOM, *PPEB_LDR_DATA_CUSTOM;

typedef struct _LDR_DATA_TABLE_ENTRY_CUSTOM {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_CUSTOM, *PLDR_DATA_TABLE_ENTRY_CUSTOM;

// Function prototypes
DLLEXPORT ULONG_PTR WINAPI ReflectiveLoader( LPVOID lpParameter );

// Hashing algorithm: ROR 13
#define HASH_KEY 13
#define LOADLIBRARYA_HASH 0xec0e4e8e
#define GETPROCADDRESS_HASH 0x7c0dfcaa
#define VIRTUALALLOC_HASH 0x91afca54
#define NTFLUSHINSTRUCTIONCACHE_HASH 0x534c0ab8
