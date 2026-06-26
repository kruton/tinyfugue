set(TFLIB_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/tf-lib")
set(TFLIB_BUILD "${CMAKE_CURRENT_BINARY_DIR}/tf-lib")
set(HELP_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/help")

file(GLOB TFLIB_FILES CONFIGURE_DEPENDS "${TFLIB_SOURCE}/*")
file(GLOB HELP_FILES CONFIGURE_DEPENDS
    "${HELP_SOURCE}/commands/*.html"
    "${HELP_SOURCE}/topics/*.html")

set(TFLIB_HTML2TF "$<TARGET_FILE:html2tf>")
set(TFLIB_MAKEHELP "$<TARGET_FILE:makehelp>")
set(TFLIB_TOOL_DEPS html2tf makehelp)
if(CMAKE_CROSSCOMPILING)
    if(NOT TF_HOST_HTML2TF AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/html2tf")
        set(TF_HOST_HTML2TF "${CMAKE_CURRENT_SOURCE_DIR}/build/html2tf")
    endif()
    if(NOT TF_HOST_MAKEHELP AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/makehelp")
        set(TF_HOST_MAKEHELP "${CMAKE_CURRENT_SOURCE_DIR}/build/makehelp")
    endif()
    if(NOT TF_HOST_HTML2TF OR NOT TF_HOST_MAKEHELP)
        message(FATAL_ERROR
            "Cross builds require native host tools for tf-lib generation. "
            "Build the native tree first, or pass -DTF_HOST_HTML2TF=/path/to/html2tf "
            "and -DTF_HOST_MAKEHELP=/path/to/makehelp.")
    endif()
    set(TFLIB_HTML2TF "${TF_HOST_HTML2TF}")
    set(TFLIB_MAKEHELP "${TF_HOST_MAKEHELP}")
    set(TFLIB_TOOL_DEPS)
endif()

add_custom_command(
    OUTPUT "${TFLIB_BUILD}/.stamp"
    COMMAND "${CMAKE_COMMAND}"
        -DSOURCE_DIR=${TFLIB_SOURCE}
        -DOUTPUT_DIR=${TFLIB_BUILD}
        -DHELP_SOURCE=${HELP_SOURCE}
        -DCHANGES_FILE=${CMAKE_CURRENT_SOURCE_DIR}/CHANGES
        -DHTML2TF=${TFLIB_HTML2TF}
        -DMAKEHELP=${TFLIB_MAKEHELP}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildTfLib.cmake"
    COMMAND "${CMAKE_COMMAND}" -E touch "${TFLIB_BUILD}/.stamp"
    DEPENDS ${TFLIB_TOOL_DEPS} ${TFLIB_FILES} ${HELP_FILES}
        "${CMAKE_CURRENT_SOURCE_DIR}/CHANGES"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildTfLib.cmake"
    VERBATIM)

add_custom_target(tf-lib ALL DEPENDS "${TFLIB_BUILD}/.stamp")
add_dependencies(tf tf-lib)
