#pragma once
#include <vector>
#include <cstdint>

class Screenshot {
public:
    // Captures screen and returns JPEG/PNG bytes
    static std::vector<uint8_t> Capture();
};
