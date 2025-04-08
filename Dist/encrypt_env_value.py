import base64
import os
import sys
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes

def pad(data):
    pad_len = 16 - (len(data) % 16)
    return data + bytes([pad_len] * pad_len)

def encrypt_aes_cbc(plaintext: str, key: bytes) -> str:
    if len(key) != 16:
        raise ValueError("Key must be 16 bytes (AES-128)")

    iv = get_random_bytes(16)
    cipher = AES.new(key, AES.MODE_CBC, iv)
    ct_bytes = cipher.encrypt(pad(plaintext.encode()))
    encrypted = iv + ct_bytes
    return "ENC:" + base64.b64encode(encrypted).decode()

def main():
    if len(sys.argv) != 3:
        print("Usage: python encrypt_env_value.py <plaintext> <key>")
        print("  <key> must be 16-character string (AES-128)")
        return

    plaintext = sys.argv[1]
    key = sys.argv[2].encode()

    result = encrypt_aes_cbc(plaintext, key)
    print(result)

if __name__ == "__main__":
    main()
