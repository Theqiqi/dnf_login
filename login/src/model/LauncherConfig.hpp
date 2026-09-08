#pragma once
#include "model/DbConfig.hpp"
#include <string>

namespace dnflogin::model {

// The complete set of parameters App needs at runtime -- the "final parameters"
// contract itself. The config layer is responsible for filling it in; App only reads
// these fields and never cares whether they came from an ini file, the environment
// or a hard-coded default.
struct LauncherConfig {
    DbConfig    db;
    std::string serverIp;   // Address handed to the game client
};

} // namespace dnflogin::model
