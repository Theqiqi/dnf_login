#pragma once
#include "../model/LauncherConfig.hpp"
#include <string>

namespace dnflogin::config {

/**
 * @brief The single configuration source: reads a file into a LauncherConfig.
 *
 * This layer is an outer sourcing adapter: it depends on model only, never on
 * core / game, and it writes nothing to the console (failures are returned via
 * err so the caller decides how to present them). Swapping in JSON, the registry
 * or a network-provided config later only requires replacing this folder; neither
 * core nor apps has to change.
 */
class Settings {
public:
    Settings() = delete;

    /**
     * @brief Reads launcher.ini from the executable's directory and fills out.
     * @param out Final parameters, fully populated on success; undefined on failure.
     * @param err Failure reason (includes the file path actually probed and the key name).
     * @return True only when every required entry was found. A missing file or a
     *         missing key returns false -- there is deliberately no fallback default.
     */
    static bool load(model::LauncherConfig& out, std::string& err);
};

} // namespace dnflogin::config
