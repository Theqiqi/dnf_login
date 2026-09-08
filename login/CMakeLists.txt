# login/CMakeLists.txt (Root File)
cmake_minimum_required(VERSION 3.20)
project(dnflogin1)

# Global Modern C++23 Compilation Standards
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Single include root for all first-party headers: sources refer to them as
# "core/X.hpp", "config/X.hpp", "model/X.hpp" -- no relative paths anywhere.
set(DNF_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 3rd Party Dependency Mapping: Local MySQL SDK Path Hooks
set(MYSQL_ROOT ${CMAKE_SOURCE_DIR}/third_party/mysql-5.0.19-winx64)
set(MYSQL_INCLUDE ${MYSQL_ROOT}/include)
set(MYSQL_LIB_DEBUG ${MYSQL_ROOT}/lib/debug)
set(MYSQL_LIB_RELEASE ${MYSQL_ROOT}/lib/opt)
set(MYSQL_DLL ${MYSQL_ROOT}/bin/libmySQL.dll)

# 3rd Party Dependency Mapping: Automated OpenSSL Module Discovery
find_package(OpenSSL REQUIRED)

# Hand over build execution flow entirely to our source code tree router
add_subdirectory(src)

# ==================== CLEAN PACKAGING HOOK ====================
# Pull in the standalone packaging profile module, keeping the root immaculate
include("${CMAKE_CURRENT_SOURCE_DIR}/scripts/Packaging.cmake")
# ==============================================================
