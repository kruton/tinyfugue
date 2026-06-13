file(READ "${HEADER}" config)

foreach(feature WIDECHAR SSL GNUTLS ZLIB ATCP GMCP OPTION102)
    if(feature STREQUAL "SSL")
        set(macro HAVE_SSL)
    elseif(feature STREQUAL "GNUTLS")
        set(macro HAVE_GNUTLS_OPENSSL_H)
    elseif(feature STREQUAL "ZLIB")
        set(macro HAVE_LIBZ)
    else()
        set(macro "${feature}")
        if(feature MATCHES "^(ATCP|GMCP|OPTION102)$")
            set(macro "ENABLE_${feature}")
        endif()
    endif()
    if(NOT config MATCHES "#define ${macro} ${EXPECT_${feature}}([\r\n]|$)")
        message(FATAL_ERROR
            "${macro} did not match expected value ${EXPECT_${feature}}")
    endif()
endforeach()

if(NOT config MATCHES "#define TERMCAP ${EXPECT_TERMCAP}([\r\n]|$)")
    message(FATAL_ERROR
        "TERMCAP did not match expected value ${EXPECT_TERMCAP}")
endif()

if(EXPECT_TERMCAP)
    if(config MATCHES "#define HARDCODE")
        message(FATAL_ERROR "HARDCODE was enabled with TERMCAP")
    endif()
elseif(NOT config MATCHES
        "#define HARDCODE ${EXPECT_HARDCODE}([\r\n]|$)")
    message(FATAL_ERROR
        "HARDCODE did not match expected value ${EXPECT_HARDCODE}")
endif()
