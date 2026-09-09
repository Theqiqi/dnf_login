# DNF Taiwan Server Pure Launcher - Architecture Blueprints

This document profiles the high-level system architecture, cryptographic handshakes, and future UI scalability guides for developers.

## 1. Directory Structural Mapping
The workspace enforces strict asset-source separation to ensure absolute decoupling:
* `src/core/`: Backend engine handling database querying, path calculation, and OpenSSL loop executions (compiles to `dnf_core.lib`).
* `src/game/`: Process initialization sandbox handling Win32 client execution (compiles to `dnf_game.lib`).
* `src/apps/`: Entry application zone. Consoles-based loops are isolated within `apps/cli/`, completely freeing up the root directory for future `apps/ui/` graphical framework injections.
* `res/`: Immutably stores compile-time assets (`resource.rc` & embedded master key pair).

## 2. Protocol & Cryptographic Handshake (The 46-Byte Token)
The custom launcher eliminates the legacy web-gateways completely. When a player logs in, the engine retrieves their unique `UID` from MySQL and structures an immutable 46-byte linear memory packet layout:
* `Bytes 0x00 - 0x03`: Unsigned 32-bit Integer (`uint32_t`) representing the Account ID, forced into Network Byte Order (Big-Endian).
* `Bytes 0x04 - 0x23`: Static 32-byte middleware routing chunk pre-filled with `0x01`.
* `Bytes 0x24 - 0x2D`: Immutable 10-byte version checksum trailer (`55 91 45 10 01 04 03 03 01 01`).

### Encryption Flow:
1. `Ticket::CreateRawPacket` builds the raw 46-byte packet in native memory buffer tracking.
2. `Crypto::EncryptAndEncode` grabs the array, signs it via an embedded RSA Private Key using `RSA_PKCS1_PADDING` (yielding a 256-byte output block).
3. The block is converted to a single-line Base64 ASCII token string (`BIO_FLAGS_BASE64_NO_NL`) and fed straight to `Launcher::launch`.

## 3. Database Specification
* **Target Schema**: `d_taiwan.accounts`
* **Account Lookup**: Checked via `accountname` string validation.
* **Password Hashing**: Synchronized explicitly via standard 32-character lowercase MD5 encryption.

## 4. Scaling To A Graphical Interface (Future Roadmap)
To introduce a native visual GUI in the future (e.g., via ImGui or Qt):
1. Create a decoupled subdirectory folder path under `src/apps/ui/`.
2. Configure a standalone `src/apps/ui/CMakeLists.txt` profile specifying an executable target (`add_executable(dnf_launcher_ui WIN32 main_ui.cpp)`).
3. Link your visual interface directly against the current functional cores (`target_link_libraries(dnf_launcher_ui PRIVATE dnf_game dnf_core)`).
4. Register the subfolder hook inside `src/apps/CMakeLists.txt` via `add_subdirectory(ui)`. Your underlying cryptography and launcher logic remain completely untouched!