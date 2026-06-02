# Developer Diagnostics and Token Verification Tools

This folder contains isolated diagnostic utilities and historical reference tokens used to reverse-engineer and verify the launcher authentication handshake protocols.

## 📂 Component Inventory
* **`decrypt.py`**: A specialized Python 3 continuous integration utility designed to run inside Linux/Kali environments. It intercepts the encrypted Base64 string from the Windows command-line arguments and applies inverse RSA modular math to expose the raw underlying 46-byte game protocol payload.
* **`publickey.pem`**: The original reference public key extracted via binary disassemblers from the closed-source game client.
* **`game.ini`**: Legacy companion configuration mapping sheet.

## 🚀 How to Run the Decryption Sandbox

### 1. Initialize Dependency Environment
The script relies on the modern Python Cryptography Toolkit (`pycryptodome`). Run the following execution command inside your current shell terminal workspace:

```bash
# For standard systems
pip install pycryptodome

# For modern Kali Linux subsystems (WSL)
pip install pycryptodome --break-system-packages
```

### 2. Execute Diagnostics Scan
Ensure your target public key token inside the `public_key_pem` variable matching block is up to date, then execute the main process thread:

```bash
python3 decrypt.py
```

### 3. Expected Output Interception
Upon a successful cryptographic handshake loop, the tool will bypass the asymmetric wrappers and dump the exact byte structure directly into the terminal window:
```text
============================================================
🎉 Success! Launcher Token Decrypted Safely.
Raw Plaintext Payload (Hex): 00000001010101...55914510010403030101
Total Packet Length: 46 Bytes
============================================================
👉 Extracted Account UID Vector: 1
============================================================
```
* Developers can read the resulting signature hex array to map out the version verification trailers (`55914510...`) required to construct new C++ launcher instances.