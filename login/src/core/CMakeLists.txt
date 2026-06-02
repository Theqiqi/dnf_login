# 1. Track files belonging strictly to the localized cryptography and DB data scope
set(CORE_FILES
    Crypto.cpp
    Database.cpp
    Path.cpp
    Ticket.cpp
)

# 2. Compile the Static Library Archive target
add_library(dnf_core STATIC 
    ${CORE_FILES}
)

# 3. Attach internal header paths so submodules can reference file mappings
target_include_directories(dnf_core PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${MYSQL_INCLUDE}"
)

# 4. Link OpenSSL dependencies directly to the core static engine
target_link_libraries(dnf_core PRIVATE
    OpenSSL::Crypto
)
