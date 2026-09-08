#include "core/Database.hpp"
#include <winsock2.h>
#include <windows.h>
#include <mysql.h>
#include <openssl/md5.h>
#include <iostream>
#include <vector>
#include <cstdio>

#pragma comment(lib, "ws2_32.lib")

namespace dnflogin::core {

std::string Database::md5Hex(const std::string& input) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), digest);

    char md5string[2 * MD5_DIGEST_LENGTH + 1];
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        std::sprintf(&md5string[i * 2], "%02x", static_cast<unsigned int>(digest[i]));
    }
    return std::string(md5string);
}

std::string Database::escapeString(const std::string& input) {
    if (!conn_ || input.empty()) return "";
    std::vector<char> buffer((input.size() * 2) + 1, '\0');
    unsigned long length = mysql_real_escape_string(conn_, buffer.data(), input.c_str(), static_cast<unsigned long>(input.size()));
    return std::string(buffer.data(), length);
}

Database::Database(const DbConfig& config) {
    conn_ = mysql_init(nullptr);
    if (!conn_) {
        std::cerr << "❌ mysql_init() initialization failure.\n";
        return;
    }

    mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8");


    if (!mysql_real_connect(conn_, config.ip.c_str(), config.user.c_str(), 
                            config.password.c_str(), nullptr, config.port, nullptr, 0)) {
        std::cerr << "❌ Connection error: " << mysql_error(conn_) << "\n";
        mysql_close(conn_);
        conn_ = nullptr;
    }
}

Database::~Database() {
    if (conn_) {
        mysql_close(conn_);
    }
}

bool Database::isValid() const {
    return conn_ != nullptr;
}

bool Database::registerUser(const std::string& uname, const std::string& passwd, std::string& uidOut) {
    if (!conn_) return false;

    std::string safeUname = escapeString(uname);
    std::string md5pass = md5Hex(passwd);

    // 显式指定库名 d_taiwan
    std::string query = "INSERT INTO d_taiwan.accounts (accountname, password) VALUES ('" +
                        safeUname + "', '" + md5pass + "')";

    if (mysql_query(conn_, query.c_str())) {
        std::cerr << "❌ Error during user registration: " << mysql_error(conn_) << "\n";
        return false;
    }

    unsigned long long autoId = mysql_insert_id(conn_);
    uidOut = std::to_string(autoId);
    return true;
}

bool Database::loginByUsername(const std::string& uname, const std::string& passwd, std::string& uidOut) {
    if (!conn_) return false;

    std::string safeUname = escapeString(uname);
    std::string md5pass = md5Hex(passwd);


    std::string query = "SELECT * FROM d_taiwan.accounts WHERE accountname='" + safeUname +
                        "' AND password='" + md5pass + "'";

    if (mysql_query(conn_, query.c_str())) {
        std::cerr << "❌ Database execution query failed: " << mysql_error(conn_) << "\n";
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn_);
    bool success = false;

    if (res) {
        if (mysql_num_rows(res) > 0) {
            MYSQL_ROW row = mysql_fetch_row(res);
            
            if (row && row[0]) {
                uidOut = row[0];
                success = true;
            }
        }
        mysql_free_result(res);
    }
    return success;
}

bool Database::loginByUid(const std::string& uid) {
    if (!conn_) return false;

    std::string safeUID = escapeString(uid);
    
    std::string query = "SELECT * FROM taiwan_login.member_login WHERE m_id=" + safeUID;

    if (mysql_query(conn_, query.c_str())) {
        std::cerr << "❌ UID check verification crashed: " << mysql_error(conn_) << "\n";
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn_);
    bool success = (res && mysql_num_rows(res) > 0);
    
    if (res) {
        mysql_free_result(res);
    }
    return success;
}

bool Database::checkUidExists(const std::string& uid) {
    return loginByUid(uid);
}

} // namespace dnflogin::core
