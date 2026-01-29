#pragma once
#include <string>
#include <vector>
#include <functional>

class FileWalker {
public:
    // Callback style or just list
    static std::vector<std::string> Scan(const std::string& startDir, const std::vector<std::string>& extensions);
};
