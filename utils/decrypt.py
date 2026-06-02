import base64
from Crypto.PublicKey import RSA

# 1. Base64 encoded Session Ticket (/ST) parameter intercepted from CLI arguments
token_base64 = (
    "Z3PQGRM08FQBbbqCq8D2MQcZ91PPHH8ccDnFXBGEfc/s08hbSu8P4D8BCfViRIKr"
    "NMUDB65ePdeZ5m3WyccLjkXArhEZV1pC+YM/w5+ynDDzjdL+23TH79q3taU9WB6i"
    "Y3GzwbAHhint8YBVyIQn5B8Ccm09odcohfVVPPBZShfFLDUFnMvAcY+uZ4Cla6j5"
    "uNM5SDUjqhhcAql9X9vJxSRYsv54sW9qa0OdSaJasxd83QWu3RNFyaWcvb9m7KYp"
    "EUrT7wmT/fN8g4tTg4lEJCqn+vKQ/OWVCv0Dly8tcvI4manqObBJts4yoYfiUt7R"
    "STgclLT5FP5+pQsMXcs/"
)

# 2. Public Key text used to verify and decrypt the launcher signature
# Note: Ensure this matching block aligns with your true companion publickey.pem text
public_key_pem = """-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2Mzieb9pITcA4f8ZlcJ6
78aFWWwQURecoJCMUdA5Cg5X6w5/umgvWmOgadWeZKTehHn08KIObx7L0x/4Rerk
ZFYPdigu7fOoJWUle7i3PU2UssyxQa56O5eAQDuQZGSuKKEqUWL7CFpXJUat0kYg
8/6o3U9goWcu4jGokjrUTsrj0++93mGssPAeB6NY+LOdkVlw4cQKm7EFmxXeMf3g
dnD461PKP6XnlVaLliA+mlEeGpqUHfQ8S4RlnxGXLP47kgCeU2OA0T1MgRGV8Ppd
uEI5G65TVZ7y7jQrNJZRj3ofVV0tjB7fNqf/Gj/qyOKBYUWVrZGU2btRHejMVkOn
IwIDAQAB
-----END PUBLIC KEY-----"""

try:
    # Decode the Base64 input parameter back to raw encrypted binary bytes
    encrypted_bytes = base64.b64decode(token_base64)
    
    # Auto-pad leading zero byte if the input was trimmed to 255 bytes by the launcher
    if len(encrypted_bytes) == 255:
        encrypted_bytes = b'\x00' + encrypted_bytes
        
    # Parse and import the cryptographic RSA Public Key context to extract N and E factors
    key = RSA.importKey(public_key_pem)
    
    # ==================== PURE MATHEMATICAL BREAKTHROUGH ====================
    # Bypass the high-level cipher.decrypt restriction entirely by using raw modular math:
    # m = c^e mod n (This natively reverses private key encryption using a public key)
    ciphertext_int = int.from_bytes(encrypted_bytes, byteorder='big')
    decrypted_int = pow(ciphertext_int, key.e, key.n)
    
    # Re-convert the huge integer back to a standardized raw data byte array
    raw_output = decrypted_int.to_bytes(256, byteorder='big')
    
    # 3. Manually strip out the RSA PKCS#1 v1.5 standard padding bytes
    # PKCS1 structure: 00 01 [continuous FF bytes] 00 [Real Payload Data]
    real_payload = b""
    if raw_output.startswith(b'\x00\x01') or raw_output.startswith(b'\x01'):
        # Locate the exact null byte separator indicating the end of the padding padding
        # Start searching from index 2 to bypass the leading indicators
        separator_idx = raw_output.find(b'\x00', 2)
        if separator_idx != -1:
            real_payload = raw_output[separator_idx + 1:]
        else:
            real_payload = raw_output
    else:
        # Fallback if padding indicator was lost during numeric conversion
        real_payload = raw_output

    # Clean any trailing null space variations if the payload structure is shorter than 46 bytes
    # But for DNF tokens, we strictly target the explicit packet size
    # =========================================================================

    # 4. Output the successfully extracted payload structure
    print("=" * 60)
    print("🎉 Success! Launcher Token Decrypted Safely via Core Math.")
    print(f"Raw Plaintext Payload (Hex): {real_payload.hex().upper()}")
    print(f"Total Packet Length: {len(real_payload)} Bytes")
    print("=" * 60)
    
    # Parse the dynamic Account ID from the fixed big-endian offset (First 4 Bytes)
    if len(real_payload) >= 4:
        uid = int.from_bytes(real_payload[0:4], byteorder='big')
        print(f"👉 Extracted Account UID Vector: {uid}")
        print("=" * 60)

except Exception as ex:
    print(f"❌ Decryption Failed: Key mismatch or payload corruption. Reason: {ex}")
