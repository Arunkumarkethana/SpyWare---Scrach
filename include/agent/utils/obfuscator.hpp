#ifndef OBFUSCATOR_HPP
#define OBFUSCATOR_HPP

#include <string>
#include <vector>

class Obfuscator {
public:
    static std::string Deobfuscate(const std::vector<unsigned char>& data, unsigned char key) {
        std::string result = "";
        for (unsigned char c : data) {
            result += (char)(c ^ key);
        }
        return result;
    }
};

#endif // OBFUSCATOR_HPP
