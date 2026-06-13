# - Check ANSI C headers
# CHECK_STDC_HEADERS ()
#
# Once done it will define STDC_HEADERS, HAVE_STDLIB_H, HAVE_STDARG_H,
# HAVE_STRING_H and HAVE_FLOAT_H, if they exist
#
MACRO (CHECK_STDC_HEADERS)
    IF (NOT CMAKE_REQUIRED_QUIET)
        MESSAGE (STATUS "Checking whether system has ANSI C header files")
    ENDIF (NOT CMAKE_REQUIRED_QUIET)

    CHECK_INCLUDE_FILES ("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)
    IF (STDC_HEADERS)
        MESSAGE (STATUS "ANSI C header files - found")
        SET (STDC_HEADERS 1 CACHE INTERNAL "System has ANSI C header files")
        SET (HAVE_STDLIB_H 1 CACHE INTERNAL "Have include stdlib.h")
        SET (HAVE_STDARG_H 1 CACHE INTERNAL "Have include stdarg.h")
        SET (HAVE_STRING_H 1 CACHE INTERNAL "Have include string.h"))
        SET (HAVE_FLOAT_H 1 CACHE INTERNAL "Have include float.h")))
    ELSE (STDC_HEADERS)
        MESSAGE (STATUS "ANSI C header files - not found")
        SET (STDC_HEADERS 0 CACHE INTERNAL "System has ANSI C header files")
    ENDIF (STDC_HEADERS)
ENDMACRO (CHECK_STDC_HEADERS)