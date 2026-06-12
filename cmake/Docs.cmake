find_program(NROFF_EXECUTABLE NAMES nroff)

if(TF_MANPAGE)
    if(NOT NROFF_EXECUTABLE)
        message(FATAL_ERROR "TF_MANPAGE=ON requires nroff.")
    endif()
    set(TF_CATMAN "${CMAKE_CURRENT_BINARY_DIR}/docs/${EXENAME}.1.catman")
    add_custom_command(
        OUTPUT "${TF_CATMAN}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${CMAKE_CURRENT_BINARY_DIR}/docs"
        COMMAND "${CMAKE_COMMAND}"
            -DNROFF=${NROFF_EXECUTABLE}
            -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/src/tf.1.nroffman
            -DOUTPUT=${TF_CATMAN}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FormatManpage.cmake"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/tf.1.nroffman"
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FormatManpage.cmake"
        VERBATIM)
    add_custom_target(doc ALL DEPENDS "${TF_CATMAN}")
endif()
