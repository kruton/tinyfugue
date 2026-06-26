# FindUTF8PROC.cmake
# Finds the utf8proc library.

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(UTF8PROC_PKG QUIET libutf8proc)
endif()

set(_UTF8PROC_LIBRARY_NAMES
    libutf8proc.a libutf8proc.so libutf8proc.dylib utf8proc.lib)

if(UTF8PROC_PKG_FOUND)
    foreach(utf8proc_include_dir ${UTF8PROC_PKG_INCLUDE_DIRS})
        if(NOT UTF8PROC_INCLUDE_DIR AND
                EXISTS "${utf8proc_include_dir}/utf8proc.h")
            set(UTF8PROC_INCLUDE_DIR "${utf8proc_include_dir}")
        endif()
    endforeach()
    foreach(utf8proc_library_dir ${UTF8PROC_PKG_LIBRARY_DIRS})
        foreach(utf8proc_library_name ${_UTF8PROC_LIBRARY_NAMES})
            if(NOT UTF8PROC_LIBRARY AND EXISTS
                    "${utf8proc_library_dir}/${utf8proc_library_name}")
                set(UTF8PROC_LIBRARY
                    "${utf8proc_library_dir}/${utf8proc_library_name}")
            endif()
        endforeach()
    endforeach()
endif()

if(NOT UTF8PROC_INCLUDE_DIR OR NOT UTF8PROC_LIBRARY)
    foreach(utf8proc_prefix ${CMAKE_PREFIX_PATH})
        if(NOT UTF8PROC_INCLUDE_DIR AND
                EXISTS "${utf8proc_prefix}/include/utf8proc.h")
            set(UTF8PROC_INCLUDE_DIR "${utf8proc_prefix}/include")
        endif()
        foreach(utf8proc_library_name ${_UTF8PROC_LIBRARY_NAMES})
            if(NOT UTF8PROC_LIBRARY AND EXISTS
                    "${utf8proc_prefix}/lib/${utf8proc_library_name}")
                set(UTF8PROC_LIBRARY
                    "${utf8proc_prefix}/lib/${utf8proc_library_name}")
            endif()
        endforeach()
    endforeach()
endif()

find_path(UTF8PROC_INCLUDE_DIR
    NAMES utf8proc.h
    HINTS ${UTF8PROC_INCLUDE_DIR}
          ${UTF8PROC_PKG_INCLUDE_DIRS}
          ${TF_EXTRA_INCLUDE_DIRS})
find_library(UTF8PROC_LIBRARY
    NAMES utf8proc
    HINTS ${UTF8PROC_PKG_LIBRARY_DIRS}
          ${TF_EXTRA_LIBRARY_DIRS})

if(UTF8PROC_PKG_VERSION)
    set(UTF8PROC_VERSION "${UTF8PROC_PKG_VERSION}")
elseif(UTF8PROC_INCLUDE_DIR AND EXISTS "${UTF8PROC_INCLUDE_DIR}/utf8proc.h")
    file(READ "${UTF8PROC_INCLUDE_DIR}/utf8proc.h" UTF8PROC_H_CONTENT)
    string(REGEX MATCH
        "#define UTF8PROC_VERSION_MAJOR[ \t]+([0-9]+)"
        _UTF8PROC_MAJOR_MATCH "${UTF8PROC_H_CONTENT}")
    if(_UTF8PROC_MAJOR_MATCH)
        set(_UTF8PROC_MAJOR "${CMAKE_MATCH_1}")
    endif()
    string(REGEX MATCH
        "#define UTF8PROC_VERSION_MINOR[ \t]+([0-9]+)"
        _UTF8PROC_MINOR_MATCH "${UTF8PROC_H_CONTENT}")
    if(_UTF8PROC_MINOR_MATCH)
        set(_UTF8PROC_MINOR "${CMAKE_MATCH_1}")
    endif()
    string(REGEX MATCH
        "#define UTF8PROC_VERSION_PATCH[ \t]+([0-9]+)"
        _UTF8PROC_PATCH_MATCH "${UTF8PROC_H_CONTENT}")
    if(_UTF8PROC_PATCH_MATCH)
        set(_UTF8PROC_PATCH "${CMAKE_MATCH_1}")
    endif()
    if(DEFINED _UTF8PROC_MAJOR AND DEFINED _UTF8PROC_MINOR AND
            DEFINED _UTF8PROC_PATCH)
        set(UTF8PROC_VERSION
            "${_UTF8PROC_MAJOR}.${_UTF8PROC_MINOR}.${_UTF8PROC_PATCH}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UTF8PROC
    REQUIRED_VARS UTF8PROC_LIBRARY UTF8PROC_INCLUDE_DIR
    VERSION_VAR UTF8PROC_VERSION)

if(UTF8PROC_FOUND AND NOT TARGET UTF8PROC::UTF8PROC)
    add_library(UTF8PROC::UTF8PROC UNKNOWN IMPORTED)
    set_target_properties(UTF8PROC::UTF8PROC PROPERTIES
        IMPORTED_LOCATION "${UTF8PROC_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${UTF8PROC_INCLUDE_DIR}")
endif()

mark_as_advanced(UTF8PROC_INCLUDE_DIR UTF8PROC_LIBRARY)
