#include "core/Path.hpp"
#include <windows.h>
#include <vector>

namespace dnflogin::core {

std::string Path::GetLauncherDir() {
    std::vector<char> buffer(MAX_PATH, '\0');
    DWORD len = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    

    while (len == buffer.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        buffer.resize(buffer.size() * 2);
        len = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }

    std::string fullPath(buffer.data(), len);
    size_t lastSlash = fullPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        return fullPath.substr(0, lastSlash); 
    }
    return fullPath;
}

std::string Path::buildGamePath(const std::string& relativeName) {

    return GetLauncherDir() + "\\" + relativeName;
}

} // namespace dnflogin::core
