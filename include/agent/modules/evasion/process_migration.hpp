#pragma once
#include <windows.h>
#include <string>
#include <vector>

class ProcessMigration {
public:
    static int FindTarget(const std::string& processName);
    static bool Inject(int pid, const std::vector<unsigned char>& shellcode);
};
