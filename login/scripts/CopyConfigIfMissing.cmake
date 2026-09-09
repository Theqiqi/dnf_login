# login/scripts/CopyConfigIfMissing.cmake
# One-shot runtime config seeding helper.
#
# Called from a POST_BUILD rule: copies the default launcher.ini template next to
# the freshly built executable, but ONLY when that file does not exist yet.
#
# Rationale: we deliberately do NOT copy-if-different (cmake -E copy_if_different).
# Once a user edits launcher.ini next to the exe (e.g. real DB password), every
# rebuild would otherwise clobber their values back to the CHANGE_ME template.
# "Copy only if missing" keeps a freshly checked-out build runnable without ever
# overwriting an already-configured runtime file.
#
# Requires (passed via -DTEMPLATE=/path/to/template -DDEST_DIR=/dir):
#   TEMPLATE - absolute path of the source template under res/config.
#   DEST_DIR - directory that will hold the runtime launcher.ini (exe dir).
if(NOT DEFINED TEMPLATE OR NOT DEFINED DEST_DIR)
    message(FATAL_ERROR "CopyConfigIfMissing.cmake requires TEMPLATE and DEST_DIR")
endif()

set(OUT_FILE "${DEST_DIR}/launcher.ini")
if(NOT EXISTS "${OUT_FILE}")
    file(COPY "${TEMPLATE}" DESTINATION "${DEST_DIR}")
    message(STATUS "Seeded default launcher.ini into ${DEST_DIR}")
else()
    message(STATUS "launcher.ini already present in ${DEST_DIR}; leaving it untouched")
endif()
