# DungeonFighterTaiwan Launcher (dnflogin)

> 🚀 **Core Technical Stack:** `C++23` | `CMake / Ninja` | `OpenSSL 3.x` | `MySQL 5.0 SDK`

English | [简体中文](./README_zh.md)

An enterprise-grade, fully modularized custom launcher for Dungeon Fighter Online (Taiwan Server Source Engine), built natively using C++23, CMake, and OpenSSL 3.x. 

This architecture completely eliminates legacy, unsecure web-gateways or PHP authenticators, utilizing a pure binary-level command-line cryptographic handshake network alignment. It delivers production-hardened security matrices alongside a highly decoupled blueprint optimized for seamless future Graphical User Interface (GUI) extensions.

---

## 📂 Project Structure & Module Navigation

The workspace enforces a strict **Asset-Source Separation** and **Application-Engine Decoupling** hierarchy to guarantee high cohesion and absolute low-coupling:

* **`src/core/`** - Low-level foundational library (`dnf_core.lib`). Manages out-of-memory RSA signatures, secure MySQL data sanitization layers, operational path calculations, and dynamic packet generation.
* **`src/game/`** - Subprocess execution library (`dnf_game.lib`). Handles pure Win32 API client execution, context management, and automatic directory alignment.
* **`src/apps/`** - Multi-application deployment entry layer. The Console Application (`cli/`) is completely isolated. Future graphical application modules can be injected via a fresh `gui/` directory without altering a single byte of your core logic.
* **`res/`** - Static compile-time resource cache. Contains the Windows `.rc` resource script and the master private key asset, securely baked directly into the executable binary headers at compile-time.
* **`tool/`** - External developer diagnostics package. Houses standalone reverse-engineering tools including your custom Kali Linux argument verification script `decrypt.py`.
* **`login/doc/`** - Comprehensive internal development whitepapers and production deployment guide sheets.

---

## 📝 Technical Documentation & Manuals

We have established dedicated engineering sheets targeting different deployment phases. Click the paths below to view the full specialized guides:

1. **👉 [Technical Specification & Cryptographic Whitepaper](./login/doc/README.md)**
   * *Deep Dive: Internal 46-byte linear payload structures, RSA asymmetric token matrices, SQL injection sanitization, and step-by-step blueprints for plugging in modern GUI framework extensions.*

2. **👉 [Compilation, Debugging, & Pipeline Automation Manual](./login/doc/BUILD_GUIDE.md)**
   * *Detailed Steps: Initializing host environments (MSVC + vcpkg), executing clean incremental Debug/Release builds, and running the continuous integration scripts to package pure releases.*

---

## 🚀 Continuous Integration & Deployment (10-Second Quickstart)

If your local environment host parameters are already initialized, you can build, optimize, filter, and package the software automatically using the automated pipelines inside the **`login/`** directory:

```bash
# 1. Navigate to the main build workspace
cd login

# 2. Compile optimized production release (Output routes to out/bin/Release/)
build-release.bat

# 3. Generate portable distribution ZIP archive (Output extracts to out/package/)
scripts/pack-zip.bat

# 4. Compile professional desktop Setup Wizard (Output extracts to out/package/)
scripts/pack-installer.bat
```

## ⚠️ Disclaimer
This project, along with its associated source files, is designed exclusively for network security research, application-layer protocol reverse-engineering, and cryptographic engineering educational workshops. Do not utilize this codebase for commercial operations or any form of unauthorized monetization.