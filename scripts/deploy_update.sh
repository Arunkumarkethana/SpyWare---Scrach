#!/bin/bash

# Blackforest Update Deployment Script
# Usage: ./scripts/deploy_update.sh

IP="43.204.32.63"
USER="ubuntu"
KEY="/Users/arunkumarkethana/blackforest/ServerThe.pem"
REMOTE_DIR="/home/ubuntu/blackforest_update"

echo "[*] Preparing update directory on AWS..."
ssh -i "$KEY" $USER@$IP "mkdir -p $REMOTE_DIR"

echo "[*] Uploading update bundle..."
scp -i "$KEY" bin/Blackforest.exe update.sig update.txt $USER@$IP:$REMOTE_DIR/

echo "[*] Starting HTTP Update Server on Port 8000..."
# Kill any existing server on 8000
ssh -i "$KEY" $USER@$IP "sudo fuser -k 8000/tcp"

# Start background HTTP server
ssh -i "$KEY" -f $USER@$IP "cd $REMOTE_DIR && nohup python3 -m http.server 8000 > /dev/null 2>&1 &"

echo ""
echo "=========================================================="
echo "[+] UPDATE SERVER IS LIVE AT claritybomma.xyz:8000"
echo "=========================================================="
echo "Content:"
echo " - Blackforest.exe (Signed Modular Binary)"
echo " - update.sig (RSA Signature)"
echo " - update.txt (Command: Update Trigger)"
echo "=========================================================="
echo "[*] Live agents will check in within 60 seconds."
