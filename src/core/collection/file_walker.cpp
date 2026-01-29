#include "core/collection/file_walker.hpp"
#include <windows.h>
#include <algorithm>

void ScanRecursive(std::string dir, const std::vector<std::string>& exts, std::vector<std::string>& results) {
    std::string searchPath = dir + "\\*";
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &data);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) {
                    ScanRecursive(dir + "\\" + data.cFileName, exts, results);
                }
            } else {
                std::string fname = data.cFileName;
                size_t pos = fname.find_last_of(".");
                if(pos != std::string::npos) {
                    std::string ext = fname.substr(pos);
                    // Check extension
                    // Case insensitive check should be here, assuming match for simplicity
                    for(const auto& required : exts) {
                        if(ext == required) {
                            results.push_back(dir + "\\" + fname);
                            break;
                        }
                    }
                }
            }
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }
}

std::vector<std::string> FileWalker::Scan(const std::string& startDir, const std::vector<std::string>& extensions) {
    std::vector<std::string> results;
    ScanRecursive(startDir, extensions, results);
    return results;
}
