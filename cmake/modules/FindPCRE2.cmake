# FindPCRE2.cmake
# Finds the PCRE2 library (8-bit)

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PCRE2_PKG QUIET libpcre2-8)
endif()

if(PCRE2_PKG_FOUND)
    if(NOT PCRE2_INCLUDE_DIRS AND EXISTS "${PCRE2_PKG_INCLUDEDIR}/pcre2.h")
        set(PCRE2_INCLUDE_DIRS "${PCRE2_PKG_INCLUDEDIR}")
    endif()
    if(NOT PCRE2_LIBRARIES)
        foreach(pcre2_libdir ${PCRE2_PKG_LIBRARY_DIRS})
            if(EXISTS "${pcre2_libdir}/libpcre2-8.a")
                set(PCRE2_LIBRARIES "${pcre2_libdir}/libpcre2-8.a")
                break()
            endif()
        endforeach()
    endif()
endif()

find_path(PCRE2_INCLUDE_DIRS
    NAMES pcre2.h
    HINTS ${PCRE2_PKG_INCLUDE_DIRS} ${TF_EXTRA_INCLUDE_DIRS}
)
find_library(PCRE2_LIBRARIES
    NAMES pcre2-8
    HINTS ${PCRE2_PKG_LIBRARY_DIRS} ${TF_EXTRA_LIBRARY_DIRS}
)

if(NOT PCRE2_INCLUDE_DIRS)
    find_path(PCRE2_INCLUDE_DIRS
        NAMES pcre2.h
        HINTS ${TF_EXTRA_INCLUDE_DIRS} ${CMAKE_PREFIX_PATH}
        PATH_SUFFIXES include
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
endif()
if(NOT PCRE2_LIBRARIES)
    find_library(PCRE2_LIBRARIES
        NAMES pcre2-8
        HINTS ${TF_EXTRA_LIBRARY_DIRS} ${CMAKE_PREFIX_PATH}
        PATH_SUFFIXES lib lib64
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
endif()

if(PCRE2_INCLUDE_DIRS AND EXISTS "${PCRE2_INCLUDE_DIRS}/pcre2.h")
    file(READ "${PCRE2_INCLUDE_DIRS}/pcre2.h" PCRE2_H_CONTENT)
    string(REGEX MATCH "#define PCRE2_MAJOR[ \t]+([0-9]+)" _MAJOR_MATCH "${PCRE2_H_CONTENT}")
    set(PCRE2_MAJOR_VERSION "${CMAKE_MATCH_1}")
    string(REGEX MATCH "#define PCRE2_MINOR[ \t]+([0-9]+)" _MINOR_MATCH "${PCRE2_H_CONTENT}")
    set(PCRE2_MINOR_VERSION "${CMAKE_MATCH_1}")
    set(PCRE2_VERSION "${PCRE2_MAJOR_VERSION}.${PCRE2_MINOR_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE2
    REQUIRED_VARS PCRE2_LIBRARIES PCRE2_INCLUDE_DIRS
    VERSION_VAR PCRE2_VERSION
)

if(PCRE2_FOUND AND NOT TARGET PCRE2::PCRE2)
    add_library(PCRE2::PCRE2 UNKNOWN IMPORTED)
    set_target_properties(PCRE2::PCRE2 PROPERTIES
        IMPORTED_LOCATION "${PCRE2_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${PCRE2_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(PCRE2_INCLUDE_DIRS PCRE2_LIBRARIES)
