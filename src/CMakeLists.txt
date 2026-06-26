configure_file(tfconfig.h.in tfconfig.h)
configure_file(tfdefs.h.in tfdefs.h)

target_sources(tflib PRIVATE
    attr.c
    command.c
    dstring.c
    expand.c
    expr.c
    globals.c
    help.c
    history.c
    keyboard.c
    macro.c
    malloc.c
    output.c
    pattern.c
    process.c
    search.c
    signals.c
    socket.c
    unicode.c
    tfio.c
    tty.c
    util.c
    variable.c
    world.c
)

if(PLATFORM_WASM)
    target_sources(tflib PRIVATE wasm_platform.c)
endif()

if(BUILD_TESTING)
    add_executable(tf_test "${CMAKE_CURRENT_SOURCE_DIR}/../tests/test_main.c")
    target_include_directories(tf_test PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(tf_test PRIVATE
        EXPECTED_TFLIBDIR="${CMAKE_INSTALL_FULL_DATAROOTDIR}/${LIBNAME}")
    target_link_libraries(tf_test PRIVATE tflib)
    if(PLATFORM_WASM)
        target_link_options(tf_test PRIVATE
            "SHELL:-sALLOW_MEMORY_GROWTH=1"
            "SHELL:-sFORCE_FILESYSTEM=1"
            "SHELL:-sEXIT_RUNTIME=1")
        if(ICU_DATA_FILE)
            target_link_options(tf_test PRIVATE "--embed-file=${ICU_DATA_FILE}@/icudt.dat")
        endif()
    endif()
    add_test(NAME unit COMMAND tf_test)

    add_test(NAME configured-features
        COMMAND "${CMAKE_COMMAND}"
            -DHEADER=${CMAKE_CURRENT_BINARY_DIR}/tfconfig.h
            -DEXPECT_WIDECHAR=${WIDECHAR}
            -DEXPECT_SSL=${HAVE_SSL}
            -DEXPECT_GNUTLS=${HAVE_GNUTLS_OPENSSL_H}
            -DEXPECT_ZLIB=${HAVE_LIBZ}
            -DEXPECT_ATCP=${ENABLE_ATCP}
            -DEXPECT_GMCP=${ENABLE_GMCP}
            -DEXPECT_OPTION102=${ENABLE_OPTION102}
            -DEXPECT_TERMCAP=${TERMCAP}
            -DEXPECT_HARDCODE=${HARDCODE}
            -DEXPECT_WASM=${PLATFORM_WASM}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/../tests/check_config.cmake")

    add_test(NAME install-layout
        COMMAND "${CMAKE_COMMAND}"
            -DBUILD_DIR=${CMAKE_BINARY_DIR}
            -DSTAGE_DIR=${CMAKE_BINARY_DIR}/test-install
            -DEXENAME=${EXENAME}
            -DLIBNAME=${LIBNAME}
            -DBINDIR=${CMAKE_INSTALL_BINDIR}
            -DDATADIR=${CMAKE_INSTALL_DATAROOTDIR}
            -DMANDIR=${CMAKE_INSTALL_MANDIR}
            -DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
            -DEXPECT_EXECUTABLE_SYMLINK=${TF_INSTALL_SYMLINK}
            -DSKIP_EXECUTABLE_SMOKE=${PLATFORM_WASM}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/../tests/verify_install.cmake")
endif()
