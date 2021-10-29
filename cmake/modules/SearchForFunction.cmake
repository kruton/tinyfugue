# Searches function in libraries
FUNCTION(SEARCH_FOR_FUNCTION func libs target result)
    CHECK_FUNCTION_EXISTS(${func} HAVE_${func}_IN_LIBC)
    IF (HAVE_${func}_IN_LIBC)
        SET(${result} 1 PARENT_SCOPE)
        RETURN()
    ENDIF()
    FOREACH(lib ${libs})
        CHECK_LIBRARY_EXISTS(${lib} ${func} "" HAVE_${func}_IN_${lib})
        IF (HAVE_${func}_IN_${lib})
            TARGET_LINK_LIBRARIES(${target} PRIVATE ${lib})
            SET(${result} 1 PARENT_SCOPE)
            RETURN()
        ENDIF()
    ENDFOREACH()
ENDFUNCTION()

# Searches first header
FUNCTION(CHECK_HEADERS_BREAK includes)
    FOREACH(header ${includes})
        CHECK_INCLUDE_FILE(${header} HAVE_${header})
        IF (HAVE_${header})
            string(TOUPPER "${header}" header_upper)
            string(REGEX REPLACE "\\." "_" header_replaced "${header_upper}")
            SET(HAVE_${header_replaced} 1 PARENT_SCOPE)
            RETURN()
        ENDIF()
    ENDFOREACH()
ENDFUNCTION()
