#include "apps/cli/App.hpp"
#include "config/Settings.hpp"
#include "core/Crypto.hpp"
#include "core/Ticket.hpp"
#include "core/Path.hpp"
#include "game/Launcher.hpp"

#include <iostream>
#include <vector>
#include <windows.h>

namespace dnflogin {

void App::run() {
    SetConsoleOutputCP(CP_UTF8);

    // Only the final parameters are consumed here: the database host, port, credentials and the
    // game server IP are no longer this layer's decision, and this layer does not care whether they
    // come from launcher.ini or anywhere else -- the file and its parsing are sealed inside config.
    model::LauncherConfig cfg;
    std::string configError;
    if (!config::Settings::load(cfg, configError)) {
        std::cerr << "[-] " << configError << "\n";
        return;
    }

    core::Database db(cfg.db);
    if (!db.isValid()) {
        std::cerr << "[-] Database connection failed. Host: " << cfg.db.ip << ":" << cfg.db.port << "\n";
        return;
    }
    std::cout << "[+] Connected to target database node: " << cfg.db.ip << ":" << cfg.db.port << "\n";

    // Primary Control Menu Router
    int choice = 0;
    std::cout << "=========================================================\n";
    std::cout << " 1 = Register Account\n";
    std::cout << " 2 = Login by Account Name\n";
    std::cout << " 3 = Direct Login by UID\n";
    std::cout << " 4 = Query Vector Check (UID Check)\n";
    std::cout << "=========================================================\n";
    std::cout << ">>> ";
    std::cin >> choice;


    switch (choice) {
        case 1:  handleRegistration(db); break;
        case 4:  handleVectorQuery(db); break;
        case 2:  handleUsernameLogin(db, cfg.serverIp); break;
        case 3:  handleUidLogin(db, cfg.serverIp); break;
        default: std::cerr << "[-] Invalid choice parameters. Exiting.\n"; break;
    }
}

// ===================================================================
// 🟢 Sub-Module Implementations 
// ===================================================================

void App::handleRegistration(core::Database& db) {
    std::string uname, passwd, newUid;
    std::cout << "Enter account name: "; std::cin >> uname;
    std::cout << "Enter secure password: "; std::cin >> passwd;
    
    if (db.registerUser(uname, passwd, newUid)) {
        std::cout << "[+] Registration complete. Allocated UID: [ " << newUid << " ]\n";
    } else {
        std::cerr << "[-] Registration aborted. Query error.\n";
    }
}

void App::handleVectorQuery(core::Database& db) {
    std::string uid;
    std::cout << "Enter target UID to query: "; std::cin >> uid;
    if (db.checkUidExists(uid)) {
        std::cout << "[+] Query vector hit: UID [ " << uid << " ] exists.\n";
    } else {
        std::cout << "[-] Query vector miss: UID [ " << uid << " ] does NOT exist.\n";
    }
}

void App::handleUsernameLogin(core::Database& db, const std::string& serverIP) {
    std::string uname, passwd, uidOut;
    std::cout << "Enter account name: "; std::cin >> uname;
    std::cout << "Enter password: "; std::cin >> passwd;
    
    if (!db.loginByUsername(uname, passwd, uidOut)) {
        std::cerr << "[-] Authentication rejected. Invalid credentials.\n";
        return;
    }
    executeGameLaunch(std::stoul(uidOut), serverIP);
}

void App::handleUidLogin(core::Database& db, const std::string& serverIP) {
    std::string uid;
    std::cout << "Enter direct target UID: "; std::cin >> uid;
    
    if (!db.loginByUid(uid)) {
        std::cerr << "[-] Verification rejected. UID does not exist.\n";
        return;
    }
    executeGameLaunch(std::stoul(uid), serverIP);
}


void App::executeGameLaunch(unsigned int targetUID, const std::string& serverIP) {
    std::cout << "[+] Bypass approved. Assembling memory tokens...\n";
    
    // 1. 调用你写好的 Ticket 模块，生成标准的 46 字节原始包
    std::vector<unsigned char> rawPacket = core::Ticket::CreateRawPacket(targetUID);
    
    // 2. 规范化静态调用：直接使用类名调用加密方法
    // 内部会自动通过 OpenSSL 的 RSA_PKCS1_PADDING 补齐原登录器的 1FFFFF...00 头部
    std::string encryptedToken = core::Crypto::EncryptAndEncode(rawPacket, "");
    if (encryptedToken.empty()) {
        std::cerr << "[-] RSA signature processing pipeline failed.\n";
        return;
    }

    std::string fullGamePath = core::Path::buildGamePath("DNF.exe");
    std::cout << "[+] Booting game client process container...\n";
    
    game::Launcher launcher;
    if (!launcher.launch(fullGamePath, encryptedToken)) {
        std::cerr << "[-] Sub-process environment setup failure.\n";
        return;
    }
    std::cout << "[+] Handshake successful. Session process established.\n";
}


} // namespace dnflogin
