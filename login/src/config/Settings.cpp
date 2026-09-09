#include "config/Settings.hpp"

#include <windows.h>

#include <cctype>
#include <fstream>
#include <map>
#include <vector>

namespace dnflogin::config {
namespace {

constexpr const char* kFileName = "launcher.ini";

// ---------------------------------------------------------------------------
// Text helpers
// ---------------------------------------------------------------------------

std::string trim(const std::string& s) {
    const char* ws = " \t\r\n\f\v";
    const size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    const size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool parsePort(const std::string& raw, unsigned int& out) {
    if (raw.empty() || raw.size() > 5) return false;
    unsigned long value = 0;
    for (const char c : raw) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        value = value * 10 + static_cast<unsigned>(c - '0');
    }
    if (value == 0 || value > 65535) return false;
    out = static_cast<unsigned int>(value);
    return true;
}

// ---------------------------------------------------------------------------
// Locating and parsing
// ---------------------------------------------------------------------------

/**
 * @brief Returns the directory holding the executable, without a trailing separator.
 *
 * Intentionally duplicates a few lines from core/Path: config depends on model only,
 * and letting it depend on core instead would break the premise that the configuration
 * source is replaceable. Dependency direction outranks removing the duplication.
 */
std::string exeDirectory() {
    std::vector<char> buffer(MAX_PATH, '\0');
    DWORD len = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

    // A return value equal to the capacity means truncation; grow and retry
    // (happens when the path is longer than MAX_PATH).
    while (len == buffer.size()) {
        buffer.resize(buffer.size() * 2, '\0');
        len = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }
    if (len == 0) return "";

    const std::string fullPath(buffer.data(), len);
    const size_t lastSlash = fullPath.find_last_of("\\/");
    if (lastSlash == std::string::npos) return "";
    return fullPath.substr(0, lastSlash);
}

/**
 * @brief Minimal ini reader: splits on the first '=' per line, lowercases keys,
 *        ignores unknown keys.
 *
 * Quote handling and inline-comment stripping are deliberately absent: they would
 * truncate `Password=p@ss;word` at the semicolon, and that failure surfaces as
 * "cannot connect to the database" -- far more expensive to diagnose than the minor
 * inconvenience of requiring comments to occupy their own line.
 * So a value is everything after '=' with only surrounding whitespace removed
 * (including the '\r' left behind by CRLF files).
 */
bool readIni(const std::string& path, std::map<std::string, std::string>& out, std::string& err) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        err = std::string("Configuration file not found: ") + path;
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        const std::string s = trim(line);
        if (s.empty() || s[0] == ';' || s[0] == '#') continue;  // blank line / full-line comment
        if (s[0] == '[') continue;                             // section header: keys are flat here

        const size_t eq = s.find('=');
        if (eq == std::string::npos) continue;                // lines without '=' are skipped

        const std::string key = toLower(trim(s.substr(0, eq)));
        if (key.empty()) continue;
        out[key] = trim(s.substr(eq + 1));                    // a later duplicate overrides earlier
    }
    return true;
}

}  // namespace

bool Settings::load(model::LauncherConfig& out, std::string& err) {
    const std::string dir = exeDirectory();
    if (dir.empty()) {
        err = "Cannot locate the directory containing the executable";
        return false;
    }

    const std::string path = dir + "\\" + kFileName;
    std::map<std::string, std::string> kv;
    if (!readIni(path, kv, err)) return false;

    // Required entries: a missing key and an empty value report differently, so that
    // "key present but left blank" is not lumped into one vague failure.
    auto require = [&](const char* key, const char* label, std::string& slot) -> bool {
        const auto it = kv.find(key);
        if (it == kv.end()) {
            err = std::string("Missing configuration key '") + label + "' (file: " + path + ")";
            return false;
        }
        if (it->second.empty()) {
            err = std::string("Configuration key '") + label + "' is empty (file: " + path + ")";
            return false;
        }
        slot = it->second;
        return true;
    };

    if (!require("host",     "Host",     out.db.ip))       return false;
    if (!require("user",     "User",     out.db.user))     return false;
    if (!require("serverip", "ServerIP", out.serverIp))    return false;

    // An empty Password is allowed: some test databases really have no password, so
    // only the presence of the line is required.
    const auto pass = kv.find("password");
    if (pass == kv.end()) {
        err = std::string("Missing configuration key 'Password' (file: ") + path
            + ", write 'Password=' with nothing after the sign if the database has no password)";
        return false;
    }
    out.db.password = pass->second;

    const auto port = kv.find("port");
    if (port == kv.end()) {
        err = std::string("Missing configuration key 'Port' (file: ") + path + ")";
        return false;
    }
    if (!parsePort(port->second, out.db.port)) {
        err = "Port is not a valid port number in the range 1-65535: " + port->second;
        return false;
    }

    return true;
}

} // namespace dnflogin::config
