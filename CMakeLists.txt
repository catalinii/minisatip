cmake_minimum_required(VERSION 3.16)

# Determine version from git. Only minisatip's own checkout: a tarball
# unpacked inside another repository would get that repository's tag.
if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
    execute_process(
        COMMAND git describe --abbrev=0 --tags
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

# A release tarball has no git metadata and would get the version "~", so a
# packager passes it with -DVERSION_STRING instead.
if(NOT VERSION_STRING)
    if(DEFINED ENV{PR})
        set(VERSION_STRING "${GIT_TAG}+PR$ENV{PR}~${GIT_REVISION}")
    elseif(GIT_TAG OR GIT_REVISION)
        set(VERSION_STRING "${GIT_TAG}~${GIT_REVISION}")
    else()
        set(VERSION_STRING "unknown")
    endif()
endif()

project(minisatip VERSION 2.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckPIESupported)

# The build type decides the optimization (it was a forced -O2, even for
# Debug). The default stays -O2 -g, and unoptimized for the sanitizer build.
get_property(MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT CMAKE_BUILD_TYPE AND NOT MULTI_CONFIG)
    if(DEBUG)
        set(DEFAULT_BUILD_TYPE "Debug")
    else()
        set(DEFAULT_BUILD_TYPE "RelWithDebInfo")
    endif()
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as default.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

set(DVBCSA_DEFAULT OFF)
set(DVBCA_DEFAULT OFF)
set(NETCVCLIENT_DEFAULT OFF)
set(CXX23_DEFAULT OFF)

option(STATIC "Build with static linking" OFF)

if(STATIC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .so .a)
endif()

# Detect available libraries for setting defaults
find_library(DVBCSA_LIBRARY dvbcsa)
find_library(CRYPTO_LIBRARY crypto)
find_library(NETCEIVER_LIBRARY netceiver)
find_path(NETCEIVER_INCLUDE_DIR libnetceiver/netcv/defs.h)
find_library(STDCPPEXP_LIBRARY NAMES stdc++exp)
find_library(SRT_LIBRARY srt)
find_path(SRT_INCLUDE_DIR srt/srt.h)
if(SRT_LIBRARY AND SRT_INCLUDE_DIR)
    set(SRT_DEFAULT ON)
else()
    set(SRT_DEFAULT OFF)
endif()

if(DVBCSA_LIBRARY)
    set(DVBCSA_DEFAULT ON)
endif()

if(CRYPTO_LIBRARY)
    set(DVBCA_DEFAULT ON)
endif()

if(NETCEIVER_LIBRARY AND NETCEIVER_INCLUDE_DIR)
    set(NETCVCLIENT_DEFAULT ON)
endif()

find_package(Threads REQUIRED)
set(MINISATIP_LIBS Threads::Threads)
if(STDCPPEXP_LIBRARY)
    set(CXX23_DEFAULT ON)
endif()

# Build options
option(DVBCSA "Enable DVB-CSA support" ${DVBCSA_DEFAULT})
option(DVBCA "Enable DVB-CA support" ${DVBCA_DEFAULT})
option(NETCVCLIENT "Enable NetCeiver client support" ${NETCVCLIENT_DEFAULT})
option(LINUXDVB "Enable Linux DVB support" ON)
option(CXX23 "Enable C++23 features (stacktrace)" ${CXX23_DEFAULT})
option(DEBUG "Enable debug/sanitizer build" OFF)
option(SRT "Enable SRT support" ${SRT_DEFAULT})
option(BUILD_TESTING "Build tests" OFF)
option(TEST_SANITIZERS "Build tests with AddressSanitizer/LeakSanitizer" ON)

# Platform detection
if(APPLE)
    set(LINUXDVB OFF)
    set(DVBCA OFF)
    set(NETCVCLIENT OFF)
endif()

if(CXX23 AND STDCPPEXP_LIBRARY)
    list(APPEND MINISATIP_LIBS ${STDCPPEXP_LIBRARY})
endif()

# Compute derived feature flags
set(DDCI OFF)
if(DVBCA AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DDCI ON)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Generate config.h
set(OPTIONS DVBCSA DVBCA NETCVCLIENT LINUXDVB DDCI CXX23 SRT)
set(CONFIG_CONTENT "// Auto-generated configuration header\n#ifndef CONFIG_H\n#define CONFIG_H\n\n")

foreach(OPT ${OPTIONS})
    if(NOT ${OPT})
        string(APPEND CONFIG_CONTENT "#ifndef DISABLE_${OPT}\n#define DISABLE_${OPT}\n#endif\n")
        message(STATUS "Option ${OPT} disabled")
    endif()
endforeach()

string(APPEND CONFIG_CONTENT "#ifndef VERSION\n#define VERSION \"${VERSION_STRING}\"\n#endif\n")
string(APPEND CONFIG_CONTENT "\n#endif // CONFIG_H\n")
# configure_file() only touches config.h when its content changes, so a
# reconfigure does not rebuild everything
file(WRITE ${CMAKE_BINARY_DIR}/config.h.tmp "${CONFIG_CONTENT}")
configure_file(${CMAKE_BINARY_DIR}/config.h.tmp ${CMAKE_BINARY_DIR}/config.h COPYONLY)

# Common compile flags
add_compile_options(
    -Wall
    -Wno-date-time
    -Wno-switch
    -fno-common
    -fno-omit-frame-pointer
    -Warray-bounds
)

if(DEBUG)
    add_compile_options(
        -ggdb
        -fsanitize=address
        -fsanitize=leak
        -fsanitize=undefined
        -fstack-protector-all
    )
    add_link_options(
        -fsanitize=address
        -fsanitize=leak
        -fsanitize=undefined
    )
    # clang links its sanitizer runtimes statically with one option
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_link_options(-static-libsan)
    elseif(NOT APPLE)
        add_link_options(
            -static-libasan
            -static-libubsan
            -static-liblsan
        )
    endif()
endif()

# A position independent executable, not -fPIC code: PIC makes every global
# call interposable. Static builds stay non-PIE.
if(NOT STATIC)
    check_pie_supported()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# The imported target brings its headers in as system headers
if(NETCVCLIENT)
    find_package(LibXml2 REQUIRED)
endif()

# Core source and library lists
set(MINISATIP_SOURCES
    src/minisatip.cpp
    src/socketworks.cpp
    src/stream.cpp
    src/adapter.cpp
    src/httpc.cpp
    src/opts.cpp
    src/utils.cpp
    src/api/symbols.cpp
    src/api/variables.cpp
    src/utils/dvb/dvb_support.cpp
    src/utils/logging/logging.cpp
    src/utils/fifo.cpp
    src/utils/ticks.cpp
    src/utils/uuid.cpp
    src/dvb.cpp
    src/pmt.cpp
    src/tables.cpp
    src/dvbapi.cpp
    src/satipc.cpp
    src/hw_descrambler.cpp
)

if(DVBCSA)
    list(APPEND MINISATIP_SOURCES src/csa.cpp)
    list(APPEND MINISATIP_LIBS ${DVBCSA_LIBRARY})
endif()
if(DVBCA)
    list(APPEND MINISATIP_SOURCES src/ca.cpp src/aes.cpp)
    list(APPEND MINISATIP_LIBS ${CRYPTO_LIBRARY} dl)
endif()
if(NETCVCLIENT)
    list(APPEND MINISATIP_SOURCES src/netceiver.cpp)
    list(APPEND MINISATIP_LIBS ${NETCEIVER_LIBRARY} LibXml2::LibXml2 lzma z)
endif()
if(DDCI)
    list(APPEND MINISATIP_SOURCES src/ddci.cpp)
endif()
if(SRT)
    list(APPEND MINISATIP_SOURCES src/srt.cpp)
    list(APPEND MINISATIP_LIBS ${SRT_LIBRARY})
endif()
if(NOT APPLE)
    list(APPEND MINISATIP_LIBS rt)
endif()

# Main executable
add_executable(minisatip ${MINISATIP_SOURCES})
target_include_directories(minisatip PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)
target_compile_options(minisatip PRIVATE -include ${CMAKE_BINARY_DIR}/config.h)
if(SRT)
    target_include_directories(minisatip SYSTEM PRIVATE ${SRT_INCLUDE_DIR})
endif()
if(NETCVCLIENT)
    target_include_directories(minisatip SYSTEM PRIVATE ${NETCEIVER_INCLUDE_DIR})
endif()
set_target_properties(minisatip PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(STATIC)
    # Full static linking with -static flag
    target_link_options(minisatip PRIVATE -static)
    # Add atomic library for static MIPS builds
    if(CMAKE_CXX_COMPILER_TARGET MATCHES "mipsel")
        list(APPEND MINISATIP_LIBS atomic)
    endif()
endif()

target_link_libraries(minisatip PRIVATE ${MINISATIP_LIBS})
install(TARGETS minisatip RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# The web interface, where -R / the working directory expects it; status.html
# is executable in git
install(DIRECTORY html/ DESTINATION ${CMAKE_INSTALL_DATADIR}/minisatip/html
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# Testing
if(BUILD_TESTING)
    enable_testing()

    set(TEST_LIBS
        crypto
        dl
        Threads::Threads
    )
    if(SRT)
        list(APPEND TEST_LIBS srt)
    endif()
    if(NOT APPLE)
        list(APPEND TEST_LIBS rt)
    endif()
    if(DVBCSA_LIBRARY)
        list(APPEND TEST_LIBS dvbcsa)
    endif()

    set(TEST_SOURCES
        tests/test_adapter.cpp
        tests/test_opts.cpp
        tests/test_ca.cpp
        tests/test_dvbapi.cpp
        tests/test_pmt.cpp
        tests/test_ddci.cpp
        tests/test_dvb.cpp
        tests/test_minisatip.cpp
        tests/test_utils.cpp
        tests/test_satipc.cpp
        tests/test_satipc_rtsp.cpp
        tests/test_socketworks.cpp
        tests/utils/test_utils_dir.cpp
        tests/test_stream.cpp
        tests/test_hw_descrambler.cpp
        tests/test_srt.cpp
        tests/test_dvb_charset.cpp
    )

    # Re-use MINISATIP_SOURCES and adjust for tests
    set(TEST_MAIN_SOURCES ${MINISATIP_SOURCES})
    list(REMOVE_ITEM TEST_MAIN_SOURCES src/netceiver.cpp)
    list(APPEND TEST_MAIN_SOURCES src/ca.cpp src/aes.cpp src/ddci.cpp)
    if(DVBCSA_LIBRARY)
        list(APPEND TEST_MAIN_SOURCES src/csa.cpp)
    endif()
    list(REMOVE_DUPLICATES TEST_MAIN_SOURCES)

    # Compile common test sources once as an object library
    add_library(test_main_lib OBJECT ${TEST_MAIN_SOURCES})
    target_include_directories(test_main_lib PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}
    )
    target_compile_definitions(test_main_lib PRIVATE
        ENIGMA
        TESTING
        DDCI_TEST
        DISABLE_STACKTRACE
        DISABLE_NETCVCLIENT
        VERSION="test"
    )
    if(NOT DVBCSA_LIBRARY)
        target_compile_definitions(test_main_lib PRIVATE DISABLE_DVBCSA)
    endif()
    target_compile_options(test_main_lib PRIVATE -fno-omit-frame-pointer)
    if(TEST_SANITIZERS)
        target_compile_options(test_main_lib PRIVATE
            -fsanitize=address
            -fsanitize=leak
        )
    endif()

    foreach(TEST_SOURCE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_SOURCE} $<TARGET_OBJECTS:test_main_lib>)
        target_include_directories(${TEST_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_BINARY_DIR}
        )
        target_compile_definitions(${TEST_NAME} PRIVATE
            ENIGMA
            TESTING
            DDCI_TEST
            DISABLE_STACKTRACE
            DISABLE_NETCVCLIENT
            VERSION="test"
        )
        if(NOT DVBCSA_LIBRARY)
            target_compile_definitions(${TEST_NAME} PRIVATE DISABLE_DVBCSA)
        endif()
        target_compile_options(${TEST_NAME} PRIVATE -fno-omit-frame-pointer)
        if(TEST_SANITIZERS)
            target_compile_options(${TEST_NAME} PRIVATE
                -fsanitize=address
                -fsanitize=leak
            )
            target_link_options(${TEST_NAME} PRIVATE
                -fsanitize=address
                -fsanitize=leak
            )
        endif()
        target_link_libraries(${TEST_NAME} PRIVATE ${TEST_LIBS})
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
message(STATUS "Version: ${VERSION_STRING}, build type: ${CMAKE_BUILD_TYPE}, FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE_UPPER}}")
