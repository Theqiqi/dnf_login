# src/CMakeLists.txt

# 1. Compile the foundational backend static library engines first
add_subdirectory(core)
add_subdirectory(game)

# 2. Configuration sourcing layer (depends on model only, never on core/game)
add_subdirectory(config)

# 3. Proceed straight into compiling the executable deployment targets
add_subdirectory(apps)
