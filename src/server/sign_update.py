#!/usr/bin/env python3
"""
Update Signer for Blackforest Auto-Update System
Signs a binary file using RSA private key
"""

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend
import sys
import os

def load_private_key(key_file="blackforest_private.pem"):
    """Load RSA private key from PEM file"""
    if not os.path.exists(key_file):
        # Try same dir as script
        script_dir = os.path.dirname(os.path.realpath(__file__))
        key_file = os.path.join(script_dir, "blackforest_private.pem")
        
    with open(key_file, 'rb') as f:
        private_key = serialization.load_pem_private_key(
            f.read(),
            password=None,
            backend=default_backend()
        )
    return private_key

def sign_file(file_path, private_key, output_sig="update.sig"):
    """Sign a file using RSA-SHA256"""
    # Read file
    with open(file_path, 'rb') as f:
        data = f.read()
    
    print(f"[*] Signing {file_path} ({len(data)} bytes)...")
    
    # Sign using PKCS#1 v1.5 padding (compatible with Windows CryptoAPI)
    signature = private_key.sign(
        data,
        padding.PKCS1v15(),
        hashes.SHA1()  # Windows CryptoAPI default
    )
    
    # Save signature (Reverse to Little-Endian for Windows CryptoAPI compatibility)
    with open(output_sig, 'wb') as f:
        f.write(signature[::-1])
    
    print(f"[+] Signature saved: {output_sig} ({len(signature)} bytes)")
    return output_sig

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 sign_update.py <file_to_sign>")
        print("Example: python3 sign_update.py Blackforest.exe")
        sys.exit(1)
    
    file_to_sign = sys.argv[1]
    
    print("=" * 60)
    print("BLACKFOREST UPDATE SIGNER")
    print("=" * 60)
    
    # Load key
    print("[*] Loading private key...")
    private_key = load_private_key()
    
    # Sign
    sign_file(file_to_sign, private_key)
    
    print("\n[+] Signing complete!")
    print("[*] Upload both files to HTTP server:")
    print(f"    - {file_to_sign}")
    print(f"    - update.sig")
