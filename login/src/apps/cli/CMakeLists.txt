# 1. 找到资源文件的绝对路径
set(LAUNCHER_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../../res/resource.rc")

# 2. Define the executable target bundle, joining the resource script directly
add_executable(dnf_launcher_cli 
    main.cpp
    App.cpp
    ${LAUNCHER_RESOURCE}  # 👈 资源必须直接挂在这里
)

# 3. Establish absolute include directories back into the 'src/' folder root
target_include_directories(dnf_launcher_cli PRIVATE 
    "${DNF_SRC_ROOT}"
)

# 4. Track target folders containing 3rd party MySQL libraries matching build configurations
target_link_directories(dnf_launcher_cli PRIVATE
    $<$<CONFIG:Debug>:${MYSQL_LIB_DEBUG}>
    $<$<CONFIG:Release>:${MYSQL_LIB_RELEASE}>
)

# 5. Link our local modularized engine blocks and libraries directly to the executable
target_link_libraries(dnf_launcher_cli PRIVATE
    dnf_game         # Resolves game process booting logic
    dnf_core         # Resolves RSA decryption & database routing layers
    dnf_config       # Resolves runtime launcher.ini into a model::LauncherConfig
    libmysql         # Resolves MySQL library links
    ws2_32           # Resolves native Windows network socket dependencies
    OpenSSL::SSL
    OpenSSL::Crypto
)

# 6. Post-Build Routine: Safely clone the dynamic libmySQL.dll asset straight to the 'out/' runtime environment
add_custom_command(TARGET dnf_launcher_cli POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${MYSQL_DLL} 
        $<TARGET_FILE_DIR:dnf_launcher_cli>
)

# 1. Instruct CMake to treat your compiled launcher binary as an installable asset
install(TARGETS dnf_launcher_cli 
        RUNTIME DESTINATION .
)

# 2. Instruct CMake to bundle the third-party runtime database library alongside it
install(FILES ${MYSQL_DLL} DESTINATION .)
