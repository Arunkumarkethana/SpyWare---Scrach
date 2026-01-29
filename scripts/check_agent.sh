#!/bin/bash
# Debug script to check agent status on Windows machine

TARGET="pss trust@192.168.0.6"

echo "=== BLACKFOREST DEBUG REPORT ==="
echo ""

echo "[1/5] Checking if Blackforest process is running..."
ssh "$TARGET" "powershell -Command \"Get-Process -Name 'Blackforest' -ErrorAction SilentlyContinue | Select-Object Id, ProcessName, Path, StartTime\""

echo ""
echo "[2/5] Checking binary location and timestamp..."
ssh "$TARGET" "powershell -Command \"Get-Item -Path '\$env:USERPROFILE\\Downloads\\Blackforest.exe' -ErrorAction SilentlyContinue | Select-Object FullName, Length, LastWriteTime\""

echo ""
echo "[3/5] Checking AppData installation..."
ssh "$TARGET" "powershell -Command \"Get-Item -Path '\$env:APPDATA\\Blackforest\\Blackforest.exe' -ErrorAction SilentlyContinue | Select-Object FullName, Length, LastWriteTime\""

echo ""
echo "[4/5] Checking network connections to C2..."
ssh "$TARGET" "powershell -Command \"Get-NetTCPConnection -RemoteAddress 192.168.0.3 -ErrorAction SilentlyContinue | Select-Object LocalPort, RemotePort, State\""

echo ""
echo "[5/5] Checking persistence (Registry)..."
ssh "$TARGET" "powershell -Command \"Get-ItemProperty -Path 'HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run' -Name 'BlackforestUpdater' -ErrorAction SilentlyContinue | Select-Object BlackforestUpdater\""

echo ""
echo "=== END OF REPORT ==="
