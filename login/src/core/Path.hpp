#pragma once
#include <string>

namespace dnflogin::core {

class Path {
public:
    Path() = delete; 


    static std::string GetLauncherDir();


    static std::string buildGamePath(const std::string& relativeName = "DNF.exe");
};

} // namespace dnflogin::core
