# src/config/CMakeLists.txt

# Configuration sourcing layer: reads a file -> fills in a model::LauncherConfig
add_library(dnf_config STATIC
    Settings.cpp
)

# Only the src root is exposed here. Note the deliberate absence of MYSQL_INCLUDE:
# this layer cannot include mysql.h, so the dependency direction is enforced by the
# build system rather than by convention.
target_include_directories(dnf_config PRIVATE
    "${DNF_SRC_ROOT}"
)

# No linking against dnf_core / dnf_game: the config layer depends only on the plain
# data types in model. GetModuleFileNameA lives in kernel32, which MSVC links by
# default, so it needs no explicit declaration.
