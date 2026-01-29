#!/usr/bin/env python3
"""
RSA Key Generator for Blackforest Auto-Update System
Generates a 2048-bit RSA keypair and exports:
1. Private Key (PEM format) - for signing updates
2. Public Key Blob (Binary format) - for embedding in agent
"""

from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend
import struct
import os

def generate_rsa_keypair():
    """Generate 2048-bit RSA key pair"""
    print("[*] Generating 2048-bit RSA keypair...")
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
        backend=default_backend()
    )
    public_key = private_key.public_key()
    return private_key, public_key

def export_private_key(private_key, filename="blackforest_private.pem"):
    """Export private key in PEM format"""
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
    return target_path

def export_public_key_blob(public_key, filename="blackforest_public.blob"):
    """Export public key blob and C++ array"""
    script_dir = os.path.dirname(os.path.realpath(__file__))
    target_path = os.path.join(script_dir, filename)
    
    # Get public numbers
    numbers = public_key.public_numbers()
    
    # Convert to bytes (big-endian)
    modulus = numbers.n.to_bytes((numbers.n.bit_length() + 7) // 8, byteorder='big')
    exponent = numbers.e.to_bytes((numbers.e.bit_length() + 7) // 8, byteorder='big')
    
    # Create a simple blob format: [modulus_len(4)][modulus][exponent_len(4)][exponent]
    blob = struct.pack('<I', len(modulus)) + modulus
    blob += struct.pack('<I', len(exponent)) + exponent
    
    with open(target_path, 'wb') as f:
        f.write(blob)
    
    print(f"[+] Public key blob saved: {target_path}")
    
    # Also generate C++ array representation
    cpp_array = "std::vector<unsigned char> pubKeyBlob = {\n    "
    for i, byte in enumerate(blob):
        cpp_array += f"0x{byte:02x}, "
        if (i + 1) % 16 == 0 and i < len(blob) - 1:
            cpp_array += "\n    "
    cpp_array = cpp_array.rstrip(", ") + "\n};"
    
    cpp_path = target_path + ".cpp"
    with open(cpp_path, 'w') as f:
        f.write(cpp_array)
    
    print(f"[+] C++ array saved: {cpp_path}")
    return target_path

if __name__ == "__main__":
    print("=" * 60)
    print("BLACKFOREST RSA KEY GENERATOR")
    print("=" * 60)
    
    # Generate keys
    private_key, public_key = generate_rsa_keypair()
    
    # Export
    export_private_key(private_key)
    export_public_key_blob(public_key)
    
    print("\n[+] Key generation complete!")
    print("[*] Use 'blackforest_private.pem' to sign updates")
    print("[*] Embed 'blackforest_public.blob.cpp' in updater.cpp")
