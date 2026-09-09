# DNF Taiwan Launcher - Process Security & Anti-Interception Analysis

This document details the reverse-engineering discoveries, operating system execution constraints, and specific defensive mechanics engineered into the C++ runtime launcher to bypass Windows 11 process blocks and secure sensitive authentication assets.

## 1. Threat Landscape & Original Launcher Flaws

Reverse-engineering the original C# launcher using `NETReactorSlayer` and `dnSpy` exposed two critical architectural vulnerabilities that rendered it unusable and insecure on modern operating systems:

```apl
[ Legacy Launcher (C#) ]
       │
       ├──> Hardcoded Assets ──> RSA Private Key exposed in plaintext plaintext metadata.
       │
       └──> Process.Start()  ──> Triggers Win11 SmartScreen / Defender Behavior Monitoring.
                                 Blocked as a suspicious unverified proxy process.
```

A. Total Cryptographic Compromise

The original launcher hardcoded the complete 2048-bit **RSA Private Key** directly inside its form class metadata as an ASCII string. Because C# compiles down to MSIL (Microsoft Intermediate Language), metadata structures remain fully intact post-compilation. Anyone with a basic decompiler could extract the private key instantly, allowing malicious actors to forge arbitrary 46-byte runtime tokens, completely bypassing billing, licensing, or launcher checks.

B. Windows 11 Process Initiation Interception

The legacy C# launcher utilized high-level standard environment wrappers:

``` csharp
Process.Start("dnf.exe", this.string_5);
```

Use code with caution.

Under Windows 11 (Build 22000+), the kernel's process creation monitoring engine applies aggressive heuristics to anonymous, unpackaged, or heavily packed (.NET Reactor) binaries. When an unverified, standalone C# application attempts to spin up a high-privilege game process loop, the operating system's SmartScreen and Windows Defender Behavior Monitoring flags it as an anonymous loader attempting a **Process Hollowing** or **Puppet Process** injection attack, resulting in silent execution termination or access denied errors.

------

## 2. Defensive Engineering in the C++ Architecture

The modern native C++ codebase mitigates these vulnerabilities down to the compiler and Win32 Subsystem levels.

A. Native Win32 Subsystem Process Mapping (`Launcher.cpp`)

Instead of utilizing high-level environment wrappers, the C++ runtime targets the absolute base kernel API: `CreateProcessA`. To bypass modern OS behavior-blocking filters, it implements two critical low-level behaviors:

1. **Mutable Parameter Buffers**: Windows `CreateProcessA` specifies that its second argument (`lpCommandLine`) must point to a mutable string buffer allocated in writeable memory space. High-level languages frequently pass pointers to read-only constant segments. The C++ launcher strictly isolates this context inside an explicit vector vector cache before handover:

   ```    cpp
   std::vector<char> mutableCmdBuffer(dynamicCmd.begin(), dynamicCmd.end());
   mutableCmdBuffer.push_back('\0');
   ```

   Use code with caution.

   This native alignment meets the OS kernel memory requirements perfectly, neutralizing modern memory access auditing flags.

2. **Explicit Working Directory Mounting**: The system forces explicit extraction of the runtime absolute folder tree context, initializing the sub-process using `workingDirectory.c_str()`. This anchors the game client safely within its native resource assets (`.npk` data files), separating it from the security context of the user's terminal space.

B. Binary-Embedded Cryptographic Asset Masking (`Crypto.cpp`)

To prevent the private key from lingering inside standard executable text segments or being discoverable via basic file scanners, the C++ launcher shifts the asset allocation out of the source text space entirely:

1. **Win32 Resource Integration**: The `privatekey.pem` payload is compiled directly into the executable binary structure utilizing the native Windows Resource Compiler (`rc.exe`) as an opaque `PEM_KEY` binary object.
2. **Volatile Memory Streaming**: During runtime initialization, the private key data is pulled straight from the read-only section of the PE (Portable Executable) map using `FindResourceA` and `LockResource`. It bypasses the file system entirely, streaming the address space cleanly into an OpenSSL abstract memory buffer (`BIO_new_mem_buf`), leaving no tracking footprint on the storage media.

------

## 3. Recommended Production Hardening Matrix

To achieve maximum protection for commercial deployments against advanced memory dump tracking and debugging tools, execute the following three-tier production security pipeline during deployment packaging:

```apl
[ Source C++ Build ]
        │
        ▼ (Phase 1: String Encapsulation)
[ skCrypter/XorStr Macro Filters ]  ──> Encrypts SQL Strings & IP parameters out of text view.
        │
        ▼ (Phase 2: Compiler Level)
[ OLLVM Control Flow Flattening ]   ──> Converts functions into an untraceable switch-case maze.
        │
        ▼ (Phase 3: Post-Compile)
[ VMProtect SDK Virtualization ]    ──> Erases X86 instructions; converts to custom bytecode.
```

Phase 1: Compile-Time String Encryption (Static String Hiding)

**Objective**: Prevent reverse engineers from using basic string extractors (e.g., `strings` command, BinText) to discover server IPs, database passwords, or RSA tokens.

- **Mechanism**: Use inline header-only macro compilers like `skCrypter` to encrypt strings with unique compile-time random XOR keys. The string exists as randomized bytes inside the binary and is only decrypted on the local CPU register space at the microsecond of execution.

Phase 2: OLLVM Code Flattening (Anti-Decompilation)

**Objective**: Destroy the structural flow representation of your authentication functions inside decompilers like IDA Pro or Ghidra.

- **Mechanism**: Map `-mllvm -fla` arguments inside your `CMakeLists.txt` build automation. This breaks down simple serial execution blocks into heavily obfuscated, multi-conditional state machines, multiplying the analysis time for reverse-engineers by orders of magnitude.

Phase 3: VMProtect SDK Virtualization Code Markers (Anti-Debugging)

**Objective**: Completely erase standard processor instructions from critical functions, forcing attackers to reverse-engineer a complex proprietary virtual execution engine.

- **Mechanism**: Encase the entire OpenSSL loading and signing process (`Crypto::EncryptAndEncode`) inside explicit SDK marker anchors:

  ```  cpp
  #include <VMProtectSDK.h>
  
  VMProtectBeginVirtualization("AuthSignaturePipeline");
  // ... Critical Private Key Data Mapping & OpenSSL Code Blocks ...
  VMProtectEnd();
  ```