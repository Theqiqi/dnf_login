# login/scripts/Packaging.cmake
# Decoupled Standalone Packaging Sub-system Module

set(CPACK_PACKAGE_NAME "DungeonFighterTaiwanLauncher")
set(CPACK_PACKAGE_VENDOR "YourNameOrStudio")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "dnf Taiwan Server Custom Pure Launcher")

# Enable both Nullsoft Installers and standard ZIP compression styles simultaneously
set(CPACK_GENERATOR "NSIS;ZIP")

# Windows NSIS Installer Shortcuts Deployment Configurations
set(CPACK_NSIS_CREATE_ICONS_EXTRA "
  CreateShortCut '\\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\dnf Launcher.lnk' '\\$INSTDIR\\\\dnf_launcher_cli.exe'
  CreateShortCut '\\$DESKTOP\\\\dnf Launcher.lnk' '\\$INSTDIR\\\\dnf_launcher_cli.exe'
")

# ==================== Dynamic External Asset Packaging Rules ====================

# Component 1: Optional Custom Game Client Binary Check
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/assets/dnf.exe")
    message(STATUS "📦 Detected manually placed dnf.exe asset. Injecting into packaging pipeline...")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/assets/dnf.exe" DESTINATION .)
endif()

# Component 2: Source Tree Public Key Deployment Mapping
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/certs/publickey.pem" DESTINATION .)


# ==================== 🟢  OpenSSL ====================
# Component 3: Automated OpenSSL Runtime Dependency Resolution Block
if(TARGET OpenSSL::Crypto)

    get_target_property(OPENSSL_CRYPTO_DLL_PATH OpenSSL::Crypto IMPORTED_LOCATION_RELEASE)
    if(NOT OPENSSL_CRYPTO_DLL_PATH)
        get_target_property(OPENSSL_CRYPTO_DLL_PATH OpenSSL::Crypto IMPORTED_LOCATION)
    endif()


    if(OPENSSL_CRYPTO_DLL_PATH MATCHES "\\.lib$")
        string(REPLACE "/lib/" "/bin/" FIXED_DLL_PATH "${OPENSSL_CRYPTO_DLL_PATH}")
        string(REGEX REPLACE "libcrypto\\.lib$" "libcrypto-3-x64.dll" FIXED_DLL_PATH "${FIXED_DLL_PATH}")
        set(OPENSSL_CRYPTO_DLL_PATH "${FIXED_DLL_PATH}")
    endif()


    if(EXISTS "${OPENSSL_CRYPTO_DLL_PATH}")
        message(STATUS "🔐 Locked OpenSSL Runtime DLL Destination: ${OPENSSL_CRYPTO_DLL_PATH}")
        install(FILES "${OPENSSL_CRYPTO_DLL_PATH}" DESTINATION .)
    else()

        set(VCPKG_BIN_DLL "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/bin/libcrypto-3-x64.dll")
        if(EXISTS "${VCPKG_BIN_DLL}")
            message(STATUS "🔐 Fallback to Vcpkg Runtime Bin Target: ${VCPKG_BIN_DLL}")
            install(FILES "${VCPKG_BIN_DLL}" DESTINATION .)
        endif()
    endif()
endif()
# ================================================================================

# Ensure the CPack initialization engine call is explicitly at the absolute end of the file
include(CPack)
