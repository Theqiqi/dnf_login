#include "Launcher.hpp"
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>

namespace dnflogin::game {

bool Launcher::launch(const std::string& gamePath, const std::string& encryptedToken) {
    if (gamePath.empty() || encryptedToken.empty()) {
        std::cerr << "❌ Launch Failed: Game path or encrypted Token cannot be empty!\n";
        return false;
    }

    // 1. Resolve the Working Directory directly from the absolute game path
    // This forces the OS to recognize the companion .npk game files located alongside DNF.exe
    std::string workingDirectory = "";
    size_t lastSlash = gamePath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        workingDirectory = gamePath.substr(0, lastSlash);
    } else {
        std::cerr << "❌ Launch Failed: Invalid game path format, failed to parse working directory!\n";
        return false;
    }

    // 2. Format the exact single-token parameter line unmasked during our extraction
    // Structure: "X:\Path\DNF.exe" <Base64_Token>
    // Note: The executable string MUST be wrapped in double quotes to satisfy the Win32 CLI parser
    std::string dynamicCmd = "\"" + gamePath + "\" " + encryptedToken;

    // CreateProcessA requires a mutable, writable character array buffer for lpCommandLine
    std::vector<char> mutableCmdBuffer(dynamicCmd.begin(), dynamicCmd.end());
    mutableCmdBuffer.push_back('\0'); // Append the null-terminator

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    std::cout << "🚀 Sending security streaming credentials and mounting the startup process...\n";

    // 3. Execute the process directly via the core subsystem API
    // Setting dwCreationFlags to 0 avoids triggering strict Anti-Cheat memory isolation flags
    BOOL success = CreateProcessA(
        gamePath.c_str(),            // Application Name Context
        mutableCmdBuffer.data(),     // Fully formatted executable parameter stream
        nullptr,                     // Process Security Attributes
        nullptr,                     // Thread Security Attributes
        FALSE,                       // Inherit Handles
        0,                           // Process Creation Flags (Standard running state)
        nullptr,                     // Environment Blocks
        workingDirectory.c_str(),    // CRITICAL: Hard-set local asset tracking directory context
        &si,                         // Startup Configuration Pointer
        &pi                          // Subprocess Handle Pipeline
    );

    if (!success) {
        DWORD win32ErrorCode = GetLastError();
        std::cerr << "❌ Main game process creation failed! System Win32 Error Code: " << win32ErrorCode << "\n";
        std::cerr << "Hint: Please check system permissions, silent antivirus blocking, or verify if DNF.exe is corrupted.\n";
        return false;
    }

    // 4. Safely detach and close references to avoid memory leaks or ghost threads on the host OS
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    return true;
}

} // namespace dnflogin::game
