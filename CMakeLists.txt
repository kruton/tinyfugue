cmake_minimum_required(VERSION 3.13)
project(tinyfugue)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(STRINGS "src/vars.mak" TFVER_LINE REGEX "^TFVER=")
string(REGEX REPLACE "^TFVER=" "" TFVER "${TFVER_LINE}")
file(STRINGS "src/main.c" TFVERSION_LINE REGEX "Fugue version")
string(REGEX REPLACE "^.*\"(.*)\".*$" "\\1" TFVERSION "${TFVERSION_LINE}")

message(NOTICE "Configuring ${TFVERSION}")

# For custom FindXXX.cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

add_executable(tf "")

include(GNUInstallDirs)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)
include(SearchForFunction)
include(SearchForHeader)

option(256COLORS "256 color support (turning off may reduce memory use)" ON)

option(ATCP "ATCP support" OFF)

option(CORE "debugging core files" OFF)

option(DEVELOPMENT "in-development features" OFF)

option(DMALLOC "debug malloc" OFF)

option(FLOAT "floating point arithmetic and functions" ON)

option(GETADDRINFO "use getaddrinfo() disabling will disable INET6" ON)

option(GMCP "GMCP support" OFF)

option(HISTORY "/recall and other history features" ON)

option(INET6 "IPv6 support" ON)

# Need to enable setting directory
option(MAILCHECK "mail checking" ON)

option(OPTION102 "102 option support" OFF)

option(PROCESS "/quote and /repeat" ON)

option(SSL "SSL support" ON)

option(SYMLINK "make a symlink NAME to the executable [NAME=BINDIR/tf]" OFF)

option(CURSES "use curses for terminal capabilities" ON)

option(VERSION "insert version number into names of installed files" OFF)

option(WIDECHAR "wide character support (UTF-8)" ON)

if (CORE)
    message(" CORE Enabled")
    set(DISABLE_CORE 0)
else (CORE)
    set(DISABLE_CORE 1)
endif (CORE)

if (DEVELOPMENT)
    message(NOTICE "Development version.")
else (DEVELOPMENT)
    string(FIND "${TFVERSION}" "alpha" IS_ALPHA_VERSION)
    if (IS_ALPHA_VERSION GREATER_EQUAL 0)
        message(NOTICE "
This is an alpha version, and so may not be fully tested on some platforms.
With the new features comes the possibility of new bugs.  If you don't want
to deal with this, get the latest stable or gamma version.

")
    endif (IS_ALPHA_VERSION GREATER_EQUAL 0)
endif (DEVELOPMENT)
if (DMALLOC)
    message(" DMALLOC Enabled")
endif (DMALLOC)

search_for_function(select "bsd" tf HAVE_SELECT)
if (NOT HAVE_SELECT)
    message(FATAL_ERROR "TF can not work without select().")
endif (NOT HAVE_SELECT)

# gethostbyname may not be in default libraries.
# QNX keeps gethostbyname in lsocket.
search_for_function(gethostbyname "bsd;nsl_s;nsl;resolv;socket" tf HAVE_GETHOSTBYNAME)

## If we haven't found connect(), look for -lsocket.
## Note: on IRIX 5, -lsocket exists, but we mustn't use its gethostbyname().
search_for_function(connect "socket" tf HAVE_CONNECT)

# TODO: enable socks detection?

search_for_function(setlocale intl tf HAVE_SETLOCALE)
if (NOT HAVE_SETLOCALE)
    message(WARNING "Locales will not be supported.")
endif (NOT HAVE_SETLOCALE)

if (SSL)
    find_package(OpenSSL)
    if (OPENSSL_FOUND)
        list(APPEND CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
        list(APPEND CMAKE_REQUIRED_LIBRARIES ${OPENSSL_SSL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES})
        check_symbol_exists(ERR_peek_error openssl/err.h HAVE_ERR_PEEK_ERROR)
        target_link_libraries(tf PRIVATE OpenSSL::SSL)
    else (OPENSSL_FOUND)
        find_package(GnuTLS)
        if (GNUTLS_FOUND)
            include_directories(${GNUTLS_INCLUDE_DIR})
            check_include_file("gnutls/openssl.h" HAVE_GNUTLS_OPENSSL_H)
            check_library_exists(gnutls-openssl SSL_free "" HAVE_GNUTLS_OPENSSL_LIB)
            if (HAVE_GNUTLS_OPENSSL_H AND HAVE_GNUTLS_OPENSSL_LIB)
                target_link_libraries(tf PRIVATE GnuTLS::GnuTLS gnutls-openssl)
                set(GNUTLS_WORKS TRUE)
            endif()
        endif(GNUTLS_FOUND)
    endif (OPENSSL_FOUND)
    if (OPENSSL_FOUND OR GNUTLS_WORKS)
        set(HAVE_SSL 1)
    else (OPENSSL_FOUND OR GNUTLS_WORKS)
        message(WARNING "SSL support will not be enabled.")
        set(SSL OFF)
    endif (OPENSSL_FOUND OR GNUTLS_WORKS)
endif (SSL)

if (CURSES)
    find_package(Curses)
    if (NOT CURSES_FOUND)
        message(FATAL_ERROR "Could not find curses library")
    endif (NOT CURSES_FOUND)
    include_directories(${CURSES_INCLUDE_DIR})
    target_link_libraries(tf PRIVATE ${CURSES_LIBRARIES})
    set(TERMCAP 1)
    set(HARDCODE 0)
else (CURSES)
    if (CYGWIN)
        set(HARDCODE "TERM_ansi")
    else (CYGWIN)
        set(HARDCODE "TERM_vt100")
    endif (CYGWIN)
endif (CURSES)

if (256COLORS)
    set(NCOLORS 256)
else(256COLORS)
    set(NCOLORS 16)
endif (256COLORS)

if (FLOAT)
    search_for_function(sqrt m tf HAVE_SQRT)
endif (FLOAT)

if (WIDECHAR)
    find_package(ICU 49.1 COMPONENTS uc i18n data REQUIRED)
    target_link_libraries(tf PRIVATE ICU::uc ICU::i18n ICU::data)
endif (WIDECHAR)

find_package(ZLIB)
if (ZLIB_FOUND)
    target_link_libraries(tf PRIVATE ZLIB::ZLIB)
    set(HAVE_LIBZ 1)
    check_include_file("zlib.h" HAVE_ZLIB_H)
endif (ZLIB_FOUND)

find_package(PCRE)
if (NOT PCRE_FOUND)
    message(FATAL_ERROR "Could not find PCRE package")
endif (NOT PCRE_FOUND)
include_directories(${PCRE_INCLUDE_DIRS})
target_link_libraries(tf PRIVATE ${PCRE_LIBRARIES})

########### headers ############

check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("memory.h" HAVE_MEMORY_H)
check_include_files("sys/select.h" HAVE_SYS_SELECT_H)

check_include_files("locale.h" HAVE_LOCALE_H)
if (NOT HAVE_LOCALE_H)
    message(WARNING "Locales will not be supported.")
endif (NOT HAVE_LOCALE_H)

check_include_files("pwd.h" HAVE_PWD_H)
if (NOT HAVE_PWD_H)
    message(WARNING "Filename '~user' expansion will not be supported.")
endif (NOT HAVE_PWD_H)

search_for_header("netinet/in.h;sys/in.h;sys/netinet/in.h" NETINET_IN_H)
if (NOT NETINET_IN_H)
    message(WARNING "I will use my own internet structures.  Good luck.")
endif (NOT NETINET_IN_H)

search_for_header("arpa/inet.h;sys/inet.h" ARPA_INET_H)

search_for_header("netdb.h;sys/netdb.h" NETDB_H)
if (NOT NETDB_H)
    message(FATAL_ERROR "can't find netdb.h")
endif (NOT NETDB_H)

check_struct_has_member("struct sockaddr" sa_len sys/socket.h HAVE_SOCKADDR_SA_LEN LANGUAGE C)

# Figure out which terminal driver to use.
check_headers_break("termios.h;termio.h;sgtty.h")

# Some brain damaged systems (Xenix) need <sys/ptem.h> for struct winsize.
check_struct_has_member("struct winsize" ws_col sys/ptem.h NEED_PTEM_H LANGUAGE C)

########### functions ############

# strftime
search_for_function(strftime "intl" tf HAVE_STRFTIME)

check_symbol_exists(h_errno "netdb.h" HAVE_H_ERRNO)

# required standard functions
check_function_exists(strstr HAVE_STRSTR)
check_function_exists(strtol HAVE_STRTOL)
if (NOT HAVE_STRSTR OR NOT HAVE_STRTOL)
    message(FATAL_ERROR "Missing required standard function.")
endif (NOT HAVE_STRSTR OR NOT HAVE_STRTOL)

# optional functions
check_function_exists(bcopy HAVE_BCOPY)
check_function_exists(bzero HAVE_BZERO)
check_function_exists(connect HAVE_CONNECT)
check_function_exists(fileno HAVE_FILENO)
check_function_exists(getcwd HAVE_GETCWD)
check_function_exists(getpwnam HAVE_GETPWNAM)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists(getwd HAVE_GETWD)
check_function_exists(hstrerror HAVE_HSTRERROR)
check_function_exists(index HAVE_INDEX)
check_function_exists(inet_aton HAVE_INET_ATON)
check_function_exists(inet_pton HAVE_INET_PTON)

check_function_exists(kill HAVE_KILL)
check_function_exists(memcpy HAVE_MEMCPY)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(raise HAVE_RAISE)
check_function_exists(setrlimit HAVE_SETRLIMIT)
check_function_exists(sigaction HAVE_SIGACTION)
check_function_exists(srand HAVE_SRAND)
check_function_exists(srandom HAVE_SRANDOM)
check_function_exists(strcasecmp HAVE_STRCASECMP)
check_function_exists(strchr HAVE_STRCHR)
check_function_exists(strcmpi HAVE_STRCMPI)
check_function_exists(strcspn HAVE_STRCSPN)
check_function_exists(strerror HAVE_STRERROR)
check_function_exists(stricmp HAVE_STRICMP)
check_function_exists(strtod HAVE_STRTOD)
check_function_exists(tzset HAVE_TZSET)
check_function_exists(waitpid HAVE_WAITPID)

check_type_size("short" SIZEOF_SHORT)
check_type_size("int" SIZEOF_INT)
check_type_size("long" SIZEOF_LONG)

check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)
check_include_files("sys/wait.h" HAVE_SYS_WAIT_H)

if (GETADDRINFO)
    check_function_exists(getaddrinfo HAVE_GETADDRINFO)
    check_function_exists(gai_strerror HAVE_GAI_STRERROR)
else (GETADDRINFO)
    set(INET6 OFF)
endif (GETADDRINFO)

if (INET6)
    check_struct_has_member("struct in6_addr" s6_addr "sys/types.h;sys/socket.h;${NETINET_IN_H}" HAVE_IN6_ADDR LANGUAGE C)
    if (HAVE_IN6_ADDR)
        set(IN6_ADDR "in6_addr")
    else (HAVE_IN6_ADDR)
        check_struct_has_member("struct in_addr6" s6_addr "sys/types.h;sys/socket.h;${NETINET_IN_H}" HAVE_IN6_ADDR LANGUAGE C)
        if (HAVE_IN6_ADDR)
            set(IN6_ADDR "in_addr6")
        endif (HAVE_IN6_ADDR)
    endif (HAVE_IN6_ADDR)
    if (NOT IN6_ADDR)
        message(WARNING "IPv6 will not be enabled.")
        set(INET6 OFF)
    endif (NOT IN6_ADDR)
endif (INET6)

########### system services ############

if (NOT MAILCHECK)
    set(MAILDIR "")
else (NOT MAILCHECK)
    message(STATUS "checking location of mail directory")
endif (NOT MAILCHECK)

### Figure out names of executable, library directory, and symlink

if (VERSION)
    set(EXENAME "tf-${TFVER}")
else (VERSION)
    set(EXENAME "tf")
endif (VERSION)

set_target_properties(tf
    PROPERTIES
        OUTPUT_NAME "${EXENAME}"
)

set(LIBNAME "${EXENAME}-lib")

### write variables

if (INET6)
    set(ENABLE_INET6 1)
endif (INET6)
if (ATCP)
    set(ENABLE_ATCP 1)
endif (ATCP)
if (GMCP)
    set(ENABLE_GMCP 1)
endif (GMCP)
if (OPTION102)
    set(ENABLE_OPTION102 1)
endif (OPTION102)
if (NOT HISTORY)
    set(NO_HISTORY 1)
endif (NOT HISTORY)
if (NOT PROCESS)
    set(NO_PROCESS 1)
endif (NOT PROCESS)
if (NOT FLOAT)
    set(NO_FLOAT 1)
endif (NOT FLOAT)

### man page

if (MANPAGE)
    set(MANPAGE_FILE "${CMAKE_INSTALL_FULL_MANDIR}/man1/${EXENAME}.1")
    set(MANTYPE "nroff")
else (MANPAGE)
    set(MANPAGE_FILE "")
    set(MANTYPE "")
endif (MANPAGE)

target_compile_options(tf PRIVATE "-DDATADIR=${CMAKE_INSTALL_FULL_DATADIR}")

include(cmake/Docs.cmake)

include(cmake/TfLib.cmake)

# Setup installation target
include(cmake/Install.cmake)

add_subdirectory(src)
