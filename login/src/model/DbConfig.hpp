#pragma once
#include <string>

namespace dnflogin::model {

// Final database connection parameters. Pure data contract: no loading logic, no IO.
// Originally defined inline in core/Database.hpp; moved out so that both apps and
// config can depend on this struct alone instead of relying on core's internals.
struct DbConfig {
    std::string ip;
    std::string user;
    std::string password;
    unsigned int port;
};

} // namespace dnflogin::model
