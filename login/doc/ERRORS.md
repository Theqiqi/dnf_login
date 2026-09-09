# Custom Runtime Launcher - Standard Status & Error Code Architecture

This document blueprints the decoupled, strongly typed status code system for the 

## 1. Subsystem Architecture Overview

To eliminate direct text bindings inside low-level subsystems (like `core::Database` or `core::Crypto`), all operations strictly yield a hexadecimal 32-bit unsigned integer integer map `uint32_t`

```
  [ 0x ]     [ 00 ]          [ 00 ]             [ 0000 ]
  Prefix   Subsystem     Severity Bit      Unique Error ID
```

- **Subsystems (Byte 3)**: `01` = Core, `02` = Database, `03` = Cryptographic, `04` = System/OS Process Launch.
- **Severity (Byte 2)**: `00` = Success/Informational, `0F` = Failure/Halt State.

------

## 2. Standard Status & Error Map Specifications

### Subsystem 01: Core System Diagnostics (`0x01XXXXXX`)

| Hexadecimal Code | Symbolic Identifier     | Operational Meaning / Root Cause                    | Recommended Interface Action      |
| ---------------- | ----------------------- | --------------------------------------------------- | --------------------------------- |
| `0x01000000`     | `STATUS_SUCCESS`        | Operation finalized successfully.                   | Proceed with standard pipeline.   |
| `0x010F0001`     | `ERR_INVALID_ARGUMENTS` | Input parsing validation failed or parameter empty. | Prompt user for clean field text. |
| `0x010F0002`     | `ERR_UNKNOWN_ROUTER_ID` | Menu index mapping selection invalid.               | Display "Option Out of Bounds".   |

### Subsystem 02: Database Cluster Operations (`0x02XXXXXX`)

| Hexadecimal Code | Symbolic Identifier            | Operational Meaning / Root Cause                            | Recommended Interface Action                     |
| ---------------- | ------------------------------ | ----------------------------------------------------------- | ------------------------------------------------ |
| `0x020F0001`     | `ERR_DB_INITIALIZATION_FAIL`   | `mysql_init` allocation crashed due to memory bounds.       | Alert "Critical Memory Error".                   |
| `0x020F0002`     | `ERR_DB_CONNECTION_LOST`       | TCP handshake failed against node IP or invalid ports.      | Dialog: "Server Offline / Check Network".        |
| `0x020F0003`     | `ERR_DB_QUERY_EXECUTION_CRASH` | Target SQL syntax errored or transaction aborted.           | Dialog: "Internal Core Query Failure".           |
| `0x020F0004`     | `ERR_DB_REGISTRATION_CONFLICT` | Target `accountname` payload matches an active cluster key. | Inline Red Text: "Username Already Taken".       |
| `0x020F0005`     | `ERR_DB_CREDENTIALS_MISMATCH`  | Password string check failed or record row empty.           | Inline Red Text: "Invalid Username or Password". |
| `0x020F0006`     | `ERR_DB_UID_NOT_WHITELISTED`   | Record missing inside `taiwan_login.member_login`.          | Dialog: "Account ID Not Activated via Admin".    |

### Subsystem 03: Asymmetric Cryptographic Engine (`0x03XXXXXX`)

| Hexadecimal Code | Symbolic Identifier            | Operational Meaning / Root Cause                             | Recommended Interface Action                                 |
| ---------------- | ------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| `0x030F0001`     | `ERR_CRYPTO_RESOURCE_MISSING`  | Win32 `FindResourceA` tracking failed to find ID 101.        | Fatal Dialog: "Launcher Damaged: Key Asset Missing".         |
| `0x030F0002`     | `ERR_CRYPTO_MEMORY_BUF_FAILED` | OpenSSL `BIO_new_mem_buf` initialization returned null.      | Fatal Dialog: "OpenSSL Low-Level Engine Allocation Failure". |
| `0x030F0003`     | `ERR_CRYPTO_PEM_PARSING_ERROR` | Embedded `privatekey.pem` content corrupted or structural change. | Fatal Dialog: "Cryptographic Certificate Mismatch".          |
| `0x030F0004`     | `ERR_CRYPTO_SIGNATURE_FAILED`  | `RSA_private_encrypt` failed on modular exponentiation.      | Fatal Dialog: "Token Seal Pipe Interrupted".                 |

### Subsystem 04: Host OS Process Control Sandbox (`0x04XXXXXX`)

| Hexadecimal Code | Symbolic Identifier           | Operational Meaning / Root Cause                            | Recommended Interface Action                              |
| ---------------- | ----------------------------- | ----------------------------------------------------------- | --------------------------------------------------------- |
| `0x040F0001`     | `ERR_GAME_PATH_NOT_FOUND`     | Path verification failed; target `dnf.exe` missing on disk. | Dialog: "Executable Missing. Move Launcher to game root". |
| `0x040F0002`     | `ERR_OS_PROCESS_INTERCEPTION` | `CreateProcessA` returned false. Win32 code locked.         | Dialog: "Launch Blocked. Disable Antivirus/SmartScreen".  |

------

## 3. C++ Application Implementation Mapping

To integrate this specification cleanly into your existing architecture without adding structural bloat, follow this programmatic pattern.

### Step A: Define Header Protocol (`src/core/Errors.hpp`)



``` cpp
#pragma once
#include <cstdint>
#include <string>

namespace dnflogin::core {

enum ResultCode : uint32_t {
    STATUS_SUCCESS                  = 0x01000000,
    ERR_INVALID_ARGUMENTS           = 0x010F0001,
    
    ERR_DB_CONNECTION_LOST          = 0x020F0002,
    ERR_DB_CREDENTIALS_MISMATCH     = 0x020F0005,
    ERR_DB_UID_NOT_WHITELISTED      = 0x020F0006,
    
    ERR_CRYPTO_PEM_PARSING_ERROR    = 0x030F0003,
    ERR_GAME_PATH_NOT_FOUND         = 0x040F0001,
    ERR_OS_PROCESS_INTERCEPTION     = 0x040F0002
};

class ErrorTranslator {
public:
    static std::string ToEnglishMessage(uint32_t code) {
        switch (code) {
            case STATUS_SUCCESS:              return "Operation completed successfully.";
            case ERR_DB_CONNECTION_LOST:      return "Database cluster unreachable. Check server IP routing.";
            case ERR_DB_CREDENTIALS_MISMATCH:  return "Invalid username or password pattern.";
            case ERR_DB_UID_NOT_WHITELISTED:   return "Target UID is valid but has not been activated.";
            case ERR_GAME_PATH_NOT_FOUND:     return "DNF.exe not discovered within the workspace folder.";
            case ERR_OS_PROCESS_INTERCEPTION: return "Process initiation denied. Check system permissions.";
            default:                          return "An unmapped internal routine error occurred.";
        }
    }
};

} // namespace dnflogin::core
```

Use code with caution.



### Step B: Decouple Function Operations (Example Refactoring)

Instead of hard-printing errors to standard out inside backend components, functions should return the raw `ResultCode`.



``` cpp
// Example adjustment inside your login function block
uint32_t Database::verifySession(const std::string& uname, const std::string& passwd, std::string& uidOut) {
    if (!conn_) return ERR_DB_CONNECTION_LOST;
    
    // ... execution logic ...
    if (mysql_num_rows(res) == 0) {
        return ERR_DB_CREDENTIALS_MISMATCH;
    }
    
    // ... everything passed ...
    return STATUS_SUCCESS;
}
```

Use code with caution.



### Step C: UI Router Processing Map

Your frontend view layer catches the numeric response code and decides whether to write it to the terminal stream or pipe it into an error dialog box element.



``` cpp
uint32_t status = db.verifySession(uname, passwd, uidOut);
if (status != core::STATUS_SUCCESS) {
    // Single point of output management - Perfect for future GUI conversions!
    std::cerr << "[-] Error Code: 0x" << std::hex << status << " -> " 
              << core::ErrorTranslator::ToEnglishMessage(status) << "\n";
    return;
}
```

Client Domain Hijacking Issue (start.dnf.tw)

- **Symptom**: The client executable `dnf.exe` has the verification domain `start.dnf.tw` hardcoded internally. Leaving this unhandled may result in network disconnection errors when 
- **Current Solution**: Since memory hook hijacking is not yet implemented in this launcher, `192.168.200.131``hosts`
- **Future Optimizations**: Implement a memory patch in `src/game/Launcher.cpp` to directly overwrite the process memory of `dnf.exe` after launching it, or integrate an automated deployment script inside `Packaging.cmake`.