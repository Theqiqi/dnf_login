#pragma once
#include "../../core/Database.hpp"
#include <string>

namespace dnflogin {

class App {
public:
    App() = default;
    ~App() = default;

    void run();

private:

    void handleRegistration(core::Database& db);
    void handleUsernameLogin(core::Database& db, const std::string& serverIP);
    void handleUidLogin(core::Database& db, const std::string& serverIP);
    void handleVectorQuery(core::Database& db);
    

    void executeGameLaunch(unsigned int targetUID, const std::string& serverIP);
};

} // namespace dnflogin
