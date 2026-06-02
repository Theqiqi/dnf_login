#pragma once
#include <string>

namespace dnflogin::game {

class Launcher {
public:
    Launcher() = default;
    ~Launcher() = default;

    /**
     * @brief Boots the game client cleanly with the verified single-parameter token structure.
     * @param gamePath Absolute location of the DNF.exe target (e.g., "C:\\Game\\DNF.exe").
     * @param encryptedToken The single-line RSA Base64 encrypted packet string.
     * @return True if the process successfully initialised on the Windows host subsystem.
     */
    bool launch(const std::string& gamePath, const std::string& encryptedToken);
};

} // namespace dnflogin::game
