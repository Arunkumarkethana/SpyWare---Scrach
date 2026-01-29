#!/bin/bash

# AWS Deployment Script for Blackforest
# Usage: ./scripts/deploy_aws.sh

IP="43.204.32.63"
USER="ubuntu"
KEY="/Users/arunkumarkethana/blackforest/ServerThe.pem"
REMOTE_DIR="/home/ubuntu/blackforest_c2"

echo "[*] Setting permissions on private key..."
chmod 400 "$KEY"

echo "[*] Preparing remote directory..."
ssh -i "$KEY" $USER@$IP "mkdir -p $REMOTE_DIR"

echo "[*] Uploading C2 Server files..."
scp -i "$KEY" src/server/* $USER@$IP:$REMOTE_DIR/

echo "[*] Setting up server environment..."
ssh -i "$KEY" $USER@$IP << EOF
    sudo apt-get update -y
    sudo apt-get install python3-pip -y
    pip3 install pycryptodome --break-system-packages
    echo "[+] Environment Ready."
EOF

echo ""
echo "=========================================================="
echo "[+] DEPLOYMENT SUCCESSFUL"
echo "=========================================================="
echo "To start the C2 server:"
echo "ssh -i $KEY $USER@$IP"
echo "cd $REMOTE_DIR && python3 server.py"
echo "=========================================================="
