#pragma once
#include "model/DbConfig.hpp"
#include <string>

// 前置声明，防止头文件污染
typedef struct st_mysql MYSQL;

namespace dnflogin::core {

// Parameter struct now lives in the model layer; kept here as an alias so existing signatures stay untouched.
using DbConfig = model::DbConfig;

class Database {
public:

    explicit Database(const DbConfig& config);
    ~Database();

    bool isValid() const;
    
    bool registerUser(const std::string& uname, const std::string& passwd, std::string& uidOut);
    bool loginByUsername(const std::string& uname, const std::string& passwd, std::string& uidOut);
    bool loginByUid(const std::string& uid);
    bool checkUidExists(const std::string& uid);

private:
    MYSQL* conn_ = nullptr;
    
    std::string escapeString(const std::string& input);
    static std::string md5Hex(const std::string& input);
};

} // namespace dnflogin::core
