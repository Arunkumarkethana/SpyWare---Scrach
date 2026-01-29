#pragma once
#include <windows.h>
#include <cstdio>

class ProcessHollowing {
public:
    bool InjectIntoSvchost(const void* shellcode, size_t size);
};
