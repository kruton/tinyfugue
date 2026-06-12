set(TFLIB_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/tf-lib")
set(TFLIB_BUILD "${CMAKE_CURRENT_BINARY_DIR}/tf-lib")

file(GLOB TFLIB_FILES CONFIGURE_DEPENDS "${TFLIB_SOURCE}/*")

add_custom_command(
    OUTPUT "${TFLIB_BUILD}/.stamp"
    COMMAND "${CMAKE_COMMAND}"
        -DSOURCE_DIR=${TFLIB_SOURCE}
        -DOUTPUT_DIR=${TFLIB_BUILD}
        -DMAKEHELP=$<TARGET_FILE:makehelp>
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildTfLib.cmake"
    COMMAND "${CMAKE_COMMAND}" -E touch "${TFLIB_BUILD}/.stamp"
    DEPENDS makehelp ${TFLIB_FILES}
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildTfLib.cmake"
    VERBATIM)

add_custom_target(tf-lib ALL DEPENDS "${TFLIB_BUILD}/.stamp")
add_dependencies(tf tf-lib)
