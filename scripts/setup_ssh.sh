#!/bin/bash
# scripts/setup_ssh.sh
# Sets up passwordless SSH auth

# Ensure project root
cd "$(dirname "$0")/.." || exit

PUBKEY_FILE="$HOME/.ssh/id_rsa.pub"
if [ ! -f "$PUBKEY_FILE" ]; then
    echo "[-] Error: SSH Key ($PUBKEY_FILE) not found."
    exit 1
fi

PUBKEY=$(cat "$PUBKEY_FILE")
REMOTE="Avengers@192.168.0.6"

echo "[*] Setting up Passwordless SSH for $REMOTE..."
echo "[*] You will be asked for the password ONE LAST TIME."

# 1. Create .ssh (ignore if exists)
ssh $REMOTE "mkdir C:\Users\Avengers\.ssh" 2>/dev/null

# 2. Append Key
ssh $REMOTE "powershell -c \"Add-Content -Path C:\Users\Avengers\.ssh\authorized_keys -Value '$PUBKEY' -Force\""

echo "[+] Done! You can now use scripts/deploy.sh without passwords."
