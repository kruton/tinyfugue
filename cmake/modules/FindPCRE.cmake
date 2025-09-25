#
# This module is designed to find/handle pcre library
#
# Requirements:
# - CMake >= 2.8.3 (for new version of find_package_handle_standard_args)
#
# The following variables will be defined for your use:
#   - PCRE_INCLUDE_DIRS     : pcre include directory
#   - PCRE_LIBRARIES        : pcre libraries
#   - PCRE_VERSION          : complete version of pcre (x.y)
#   - PCRE_MAJOR_VERSION    : major version of pcre
#   - PCRE_MINOR_VERSION    : minor version of pcre
#
# How to use:
#   1) Copy this file in the root of your project source directory
#   2) Then, tell CMake to search this non-standard module in your project directory by adding to your CMakeLists.txt:
#        set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
#   3) Finally call find_package(PCRE) once
#
# Here is a complete sample to build an executable:
#
#   set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
#
#   find_package(PCRE REQUIRED) # Note: name is case sensitive
#
#   include_directories(${PCRE_INCLUDE_DIRS})
#   add_executable(myapp myapp.c)
#   target_link_libraries(myapp ${PCRE_LIBRARIES})
#


#=============================================================================
# Copyright (c) 2015, julp
#
# Distributed under the OSI-approved BSD License
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#=============================================================================

########## Private ##########
if(NOT DEFINED PCRE_PUBLIC_VAR_NS)
    set(PCRE_PUBLIC_VAR_NS "PCRE")
endif(NOT DEFINED PCRE_PUBLIC_VAR_NS)
if(NOT DEFINED PCRE_PRIVATE_VAR_NS)
    set(PCRE_PRIVATE_VAR_NS "_${PCRE_PUBLIC_VAR_NS}")
endif(NOT DEFINED PCRE_PRIVATE_VAR_NS)

function(pcre_debug _VARNAME)
    if(${PCRE_PUBLIC_VAR_NS}_DEBUG)
        if(DEFINED ${PCRE_PUBLIC_VAR_NS}_${_VARNAME})
            message("${PCRE_PUBLIC_VAR_NS}_${_VARNAME} = ${${PCRE_PUBLIC_VAR_NS}_${_VARNAME}}")
        else(DEFINED ${PCRE_PUBLIC_VAR_NS}_${_VARNAME})
            message("${PCRE_PUBLIC_VAR_NS}_${_VARNAME} = <UNDEFINED>")
        endif(DEFINED ${PCRE_PUBLIC_VAR_NS}_${_VARNAME})
    endif(${PCRE_PUBLIC_VAR_NS}_DEBUG)
endfunction(pcre_debug)

########## Public ##########

# pkg-config --list-all | grep -i pcre
# libpcreposix                   libpcreposix - PCREPosix - Posix compatible interface to libpcre
# libpcre16                      libpcre16 - PCRE - Perl compatible regular expressions C library with 16 bit character support
# libpcre                        libpcre - PCRE - Perl compatible regular expressions C library with 8 bit character support
# libpcre32                      libpcre32 - PCRE - Perl compatible regular expressions C library with 32 bit character support
# libpcrecpp                     libpcrecpp - PCRECPP - C++ wrapper for PCRE

# Try to find pcre with the help of pcre-config
find_program(${PCRE_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE pcre-config)
if(${PCRE_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE)
    execute_process(COMMAND ${${PCRE_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --cflags OUTPUT_VARIABLE ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
        set(${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS "/usr/include/") # pcre-config output nothing if value is "/usr/include"
    else(NOT ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
        string(REGEX REPLACE "^-I" "" ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS ${${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS}) # strip leading -I
    endif(NOT ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
    if(${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
        # version and its component
        execute_process(COMMAND ${${PCRE_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --version OUTPUT_VARIABLE ${PCRE_PUBLIC_VAR_NS}_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REGEX MATCHALL "[0-9]+" ${PCRE_PRIVATE_VAR_NS}_VERSION_PARTS ${${PCRE_PUBLIC_VAR_NS}_VERSION})
        list(GET ${PCRE_PRIVATE_VAR_NS}_VERSION_PARTS 0 ${PCRE_PUBLIC_VAR_NS}_MAJOR_VERSION)
        list(GET ${PCRE_PRIVATE_VAR_NS}_VERSION_PARTS 1 ${PCRE_PUBLIC_VAR_NS}_MINOR_VERSION)
        # library
        execute_process(COMMAND ${${PCRE_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE} --libs OUTPUT_VARIABLE ${PCRE_PRIVATE_VAR_NS}_LIBRARY_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(NOT ${PCRE_PRIVATE_VAR_NS}_LIBRARY_DIR)
            set(${PCRE_PRIVATE_VAR_NS}_LIBRARY_DIR "/usr/lib/") # pcre-config output nothing if value is "/usr/lib"
        else(NOT ${PCRE_PRIVATE_VAR_NS}_LIBRARY_DIR)
            string(REGEX REPLACE "-L([^ ]+)" "\\1" ${PCRE_PRIVATE_VAR_NS}_LIBRARY_DIR ${${PCRE_PRIVATE_VAR_NS}_LIBRARY_DIR})
        endif(NOT ${PCRE_PRIVATE_VAR_NS}_LIBRARY_DIR)
        find_library(
            ${PCRE_PUBLIC_VAR_NS}_LIBRARIES
            NAMES pcre
            PATHS ${PCRE_PRIVATE_VAR_NS}_LIBRARY_DIR
        )
    endif(${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
endif(${PCRE_PUBLIC_VAR_NS}_CONFIG_EXECUTABLE)

# Else try to find pcre ourselves without the help of pcre-config
if(NOT ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
    find_path(
        ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS
        NAMES pcre.h
        PATH_SUFFIXES "include"
    )
    if(${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
        find_library(
            ${PCRE_PUBLIC_VAR_NS}_LIBRARIES
            NAMES pcre
        )
        file(READ "${${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS}/pcre.h" ${PCRE_PRIVATE_VAR_NS}_H_CONTENT)
        string(REGEX REPLACE ".*# *define +PCRE_MAJOR +([0-9]+).*" "\\1" ${PCRE_PUBLIC_VAR_NS}_MAJOR_VERSION ${${PCRE_PRIVATE_VAR_NS}_H_CONTENT})
        string(REGEX REPLACE ".*# *define +PCRE_MINOR +([0-9]+).*" "\\1" ${PCRE_PUBLIC_VAR_NS}_MINOR_VERSION ${${PCRE_PRIVATE_VAR_NS}_H_CONTENT})
        set(${PCRE_PUBLIC_VAR_NS}_VERSION "${${PCRE_PUBLIC_VAR_NS}_MAJOR_VERSION}.${${PCRE_PUBLIC_VAR_NS}_MINOR_VERSION}")
    endif(${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)

    include(FindPackageHandleStandardArgs)
    if(${PCRE_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${PCRE_PUBLIC_VAR_NS}_FIND_QUIETLY)
        find_package_handle_standard_args(
            ${PCRE_PUBLIC_VAR_NS}
            REQUIRED_VARS ${PCRE_PUBLIC_VAR_NS}_LIBRARIES ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS
            VERSION_VAR ${PCRE_PUBLIC_VAR_NS}_VERSION
        )
    else(${PCRE_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${PCRE_PUBLIC_VAR_NS}_FIND_QUIETLY)
        find_package_handle_standard_args(${PCRE_PUBLIC_VAR_NS} "pcre not found" ${PCRE_PUBLIC_VAR_NS}_LIBRARIES ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
    endif(${PCRE_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${PCRE_PUBLIC_VAR_NS}_FIND_QUIETLY)
endif(NOT ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)

if(${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
    set(${PCRE_PUBLIC_VAR_NS}_FOUND TRUE)
else(${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)
    # We have failed
    if(${PCRE_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${PCRE_PUBLIC_VAR_NS}_FIND_QUIETLY)
        message(FATAL_ERROR "Could not find pcre include directory")
    endif(${PCRE_PUBLIC_VAR_NS}_FIND_REQUIRED AND NOT ${PCRE_PUBLIC_VAR_NS}_FIND_QUIETLY)
endif(${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS)

mark_as_advanced(
    ${PCRE_PUBLIC_VAR_NS}_INCLUDE_DIRS
    ${PCRE_PUBLIC_VAR_NS}_LIBRARIES
)

# IN (args)
pcre_debug("FIND_REQUIRED")
pcre_debug("FIND_QUIETLY")
pcre_debug("FIND_VERSION")
# OUT
# Linking
pcre_debug("INCLUDE_DIRS")
pcre_debug("LIBRARIES")
# Version
pcre_debug("MAJOR_VERSION")
pcre_debug("MINOR_VERSION")
pcre_debug("VERSION")
