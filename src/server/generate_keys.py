#!/usr/bin/env python3
"""
Standardized RSA Key Generator for Blackforest
Generates a 2048-bit RSA keypair and exports a valid Windows PUBLICKEYBLOB.
"""

from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend
import struct
import os

def generate_rsa_keypair():
    print("[*] Generating 2048-bit RSA keypair...")
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
        backend=default_backend()
    )
    public_key = private_key.public_key()
    return private_key, public_key

def export_private_key(private_key, filename="blackforest_private.pem"):
    script_dir = os.path.dirname(os.path.realpath(__file__))
    target_path = os.path.join(script_dir, filename)
    pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption()
    )
    with open(target_path, 'wb') as f:
        f.write(pem)
    print(f"[+] Private key saved: {target_path}")

def export_public_key_blob(public_key, filename="blackforest_public.blob"):
    script_dir = os.path.dirname(os.path.realpath(__file__))
    target_path = os.path.join(script_dir, filename)
    
    numbers = public_key.public_numbers()
    
    # 1. PUBLICKEYBLOB Header
    # bType = 0x06 (PUBLICKEYBLOB), bVersion = 0x02, reserved = 0, aiKeyAlg = 0x0000a400 (CALG_RSA_KEYX)
    header = struct.pack('<BBHI', 0x06, 0x02, 0, 0x0000a400)
    
    # 2. RSAPUBKEY Structure
    # magic = "RSA1", bitlen = 2048, pubexp = 65537
    rsapubkey = struct.pack('<4sII', b'RSA1', 2048, numbers.e)
    
    # 3. Modulus (Must be Little-Endian for CryptoAPI)
    modulus = numbers.n.to_bytes(256, byteorder='little')
    
    blob = header + rsapubkey + modulus
    
    with open(target_path, 'wb') as f:
        f.write(blob)
    print(f"[+] Public key blob saved: {target_path}")
    
    # Generate C++ array
    cpp_array = "std::vector<unsigned char> pubKeyBlob = {\n    "
    for i, byte in enumerate(blob):
        cpp_array += f"0x{byte:02x}, "
        if (i + 1) % 16 == 0 and i < len(blob) - 1:
            cpp_array += "\n    "
    cpp_array = cpp_array.rstrip(", ") + "\n};"
    
    with open(target_path + ".cpp", 'w') as f:
        f.write(cpp_array)
    print(f"[+] C++ array saved: {target_path}.cpp")

if __name__ == "__main__":
    private_key, public_key = generate_rsa_keypair()
    export_private_key(private_key)
    export_public_key_blob(public_key)
