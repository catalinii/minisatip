cmake_minimum_required(VERSION 3.16)

# Determine version from git
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

if(DEFINED ENV{PR})
    set(VERSION_STRING "${GIT_TAG}+PR$ENV{PR}~${GIT_REVISION}")
else()
    set(VERSION_STRING "${GIT_TAG}~${GIT_REVISION}")
endif()

if(NOT VERSION_STRING)
    set(VERSION_STRING "unknown")
endif()

project(minisatip VERSION 2.0 LANGUAGES CXX)

set(DVBCSA_DEFAULT OFF)
set(DVBCA_DEFAULT OFF)
set(NETCVCLIENT_DEFAULT OFF)
set(CXX23_DEFAULT OFF)

# Detect available libraries for setting defaults
find_library(DVBCSA_LIBRARY dvbcsa)
find_library(CRYPTO_LIBRARY crypto)
find_library(NETCEIVER_LIBRARY netceiver)
find_path(NETCEIVER_INCLUDE_DIR libnetceiver/netcv/defs.h)
find_library(STDCPPEXP_LIBRARY NAMES stdc++exp)

if(DVBCSA_LIBRARY)
    set(DVBCSA_DEFAULT ON)
endif()

if(CRYPTO_LIBRARY)
    set(DVBCA_DEFAULT ON)
endif()

if(NETCEIVER_LIBRARY AND NETCEIVER_INCLUDE_DIR)
    set(NETCVCLIENT_DEFAULT ON)
endif()

if(STDCPPEXP_LIBRARY)
    set(CXX23_DEFAULT ON)
    target_link_libraries(your_target PRIVATE ${STDCPPEXP_LIBRARY})
endif()
# Build options
option(DVBCSA "Enable DVB-CSA support" ${DVBCSA_DEFAULT})
option(DVBCA "Enable DVB-CA support" ${DVBCA_DEFAULT})
option(NETCVCLIENT "Enable NetCeiver client support" ${NETCVCLIENT_DEFAULT})
option(LINUXDVB "Enable Linux DVB support" ON)
option(CXX23 "Enable C++23 features (stacktrace)" ${CXX23_DEFAULT})
option(STATIC "Build with static linking" OFF)
option(DEBUG "Enable debug/sanitizer build" OFF)
option(BUILD_TESTING "Build tests" OFF)

# Platform detection
if(APPLE)
    set(LINUXDVB OFF)
    set(DVBCA OFF)
    set(NETCVCLIENT OFF)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Common compile flags
add_compile_options(
    -Wall
    -Wno-date-time
    -Wno-switch
    -fPIC
    -fno-common
    -fno-omit-frame-pointer
    -Warray-bounds
)

if(DEBUG)
    add_compile_options(
        -g
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
    if(NOT APPLE)
        add_link_options(
            -static-libasan
            -static-libubsan
            -static-liblsan
        )
    endif()
else()
    add_compile_options(-O2)
endif()

# Add subdirectories
add_subdirectory(src)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "Version: ${VERSION_STRING}, FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_RELEASE}")
