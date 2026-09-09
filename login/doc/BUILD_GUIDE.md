# Compilation, Debugging, and Packaging Automation Guide

This deployment guide walks developers through local compilation steps, debugging workflows, and standard output extraction pipelines using MSVC and CPack.

## 1. Prerequisites & Environment Initialization
* **Operating System**: Windows 10/11 x64 (Build environments mapped via Kali WSL layer).
* **Compiler Sub-system**: Microsoft Visual Studio 2022 (v17.12+ for C++23 native integrations).
* **Build Generator**: Ninja Build Engine.
* **Dependency Toolchain**: Microsoft vcpkg. Ensure `vcpkg.json` includes `openssl` configuration manifests.

## 2. Native Compilation Pipelines
The project uses `CMakePresets.json` to route target dependencies. Execute compilation directly from the project root using the provided batch shortcuts:

### A. Development Debug Compilation
Run **`build-debug.bat`** from the project root.
* Targets: Outputs raw execution files into `out/build/debug/`.
* Features: Compiles with debugging code maps (`/Zi`, `/Od`) and includes heavy system tracking hooks (`/RTC1`). Local static archives (`dnf_core.lib`) include symbol maps for active breakpoints.

### B. Production Release Optimization
Run **`build-release.bat`** from the project root.
* Targets: Outputs optimized executable files inside `out/build/release/`.
* Features: Full optimization flags triggered (`/O2`, `/Ob2`, `/DNDEBUG`). Symbols are stripped to limit reverse-engineering visibility.

## 3. Dual-Generator Automated Packaging (Continuous Integration)
Packaging scripts are completely decoupled from compilation files and reside inside the `scripts/` folder tree. **Always run build-release.bat prior to running the packaging sequences.**

### A. ZIP Distribution Archive (Lightweight Sandbox Releases)
Run **`scripts/pack-zip.bat`**.
* This triggers CPack utilizing the standard ZIP compression algorithm modules.
* It dynamically strips out local build intermediate static archives (`.lib`) [INDEX].
* It targets your active vcpkg environments, pulls the release runtime `libcrypto-3-x64.dll`, maps your public security tokens (`publickey.pem`), and grabs manual external `res/assets/dnf.exe` files.
* Final Flat Package Output location: **`out/package/DungeonFighterTaiwanLauncher-1.0.0-win64.zip`**

### B. NSIS Setup Wizard (Production Customer Installations)
Run **`scripts/pack-installer.bat`**.
* This triggers the CPack NSIS backend loop. (*Requires Nullsoft Scriptable Install System pre-installed on the host PC*).
* Packages all operational binaries directly inside a standalone professional installer layout wizard.
* Automatically registers Desktop and Start Menu shortcut links upon deployment on the player's OS.
* Final Package Output location: **`out/package/DungeonFighterTaiwanLauncher-1.0.0-win64.exe`**

## 4. Key Rotation Maintenance Routine
If your server's access certificate parameters change:
1. Replace your text tokens inside `res/certs/privatekey.pem` and `publickey.pem` files on disk.
2. Delete the temporary intermediate folder at `out/` to clear out Ninja resource caching parameters.
3. Re-run your `build-release.bat` script. The Windows resource compiler (`rc.exe`) will automatically bake the fresh private configurations directly into the binary assembly data headers at compile-time.

## 5. Cryptographic Key Rotation & Maintenance Routine

The asymmetric RSA signature pipeline relies on a synchronized 2048-bit keypair. If a key compromise occurs, or you deploy onto a clean CentOS environment, you must execute a manual key rotation sequence.

### Step A: Generate New 2048-Bit RSA Keypair via OpenSSL
Execute the following commands sequentially within your shell terminal (or inside your Kali WSL subsystem environment):

```bash
# 1. Generate an unencrypted 2048-bit RSA Private Key in standard PKCS#1 PEM format
openssl genrsa -out privatekey.pem 2048

# 2. Extract the matching cryptographic RSA Public Key from the private key string
openssl rsa -in privatekey.pem -pubout -out publickey.pem
```

### Step B: Manual Source Tree Replacement & Local Compilation
1. Take the newly generated `privatekey.pem` and `publickey.pem` files.
2. **Manually copy and overwrite** them directly into your repository asset folder at: `login/res/certs/`.
3. Wipe out your local cache directory entirely by manually deleting the `login/out/` folder.
4. Run your master build automation script: **`build-release.bat`**. 
   *(The Windows resource compiler `rc.exe` will automatically load your fresh privatekey data out of memory and bake it permanently into the `dnf_launcher_cli.exe` executable binary text segment).*

### Step C: Server-Side Public Key Synchronization
1. Establish an SSH connection to your CentOS gaming backend via WinSCP or FileZilla.
2. Upload the newly generated **`publickey.pem`** (from your `login/res/certs/` folder) straight into your server runtime directory at: `/home/neople/game/publickey.pem` (Overwriting the pre-existing server key).
3. Open your CentOS terminal via PuTTY or secure shell, and restart the channels engine to refresh memory buffers:
   ```bash
   cd /home/neople/game/
   chmod +x publickey.pem
   ./stop
   ./run
   ```