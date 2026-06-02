# src/game/CMakeLists.txt

# 1. Compile the raw Win32 CreateProcess wrapper logic independently
add_library(dnf_game STATIC 
    Launcher.cpp
)

# 2. Expose standard parent inclusion mappings
target_include_directories(dnf_game PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)
