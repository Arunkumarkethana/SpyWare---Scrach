#!/bin/bash
# scripts/deploy.sh
# One-Click Deployment (Upload + Execute)

# Ensure project root
cd "$(dirname "$0")/.." || exit

# Config
REMOTE="Avengers@192.168.0.6"
TARGET_DIR="C:/Users/Avengers/Downloads"
LOCAL_BIN="bin/Blackforest.exe"

if [ ! -f "$LOCAL_BIN" ]; then
    echo "[-] Error: $LOCAL_BIN not found. Run scripts/build.sh first."
    exit 1
fi

echo "[*] Killing old process..."
ssh $REMOTE "Stop-Process -Name 'Blackforest' -Force -ErrorAction SilentlyContinue"

echo "[*] Uploading Blackforest..."
scp "$LOCAL_BIN" $REMOTE:$TARGET_DIR/Blackforest.exe

echo "[*] Executing Agent..."
ssh $REMOTE "Start-Process $TARGET_DIR/Blackforest.exe"

echo "[+] Deployed! Check your C2."
