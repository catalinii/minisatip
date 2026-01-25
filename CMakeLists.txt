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

set(MINISATIP_LIBS pthread)
if(STDCPPEXP_LIBRARY)
    set(CXX23_DEFAULT ON)
    list(APPEND MINISATIP_LIBS ${STDCPPEXP_LIBRARY})
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
set(OPTIONS DVBCSA DVBCA NETCVCLIENT LINUXDVB DDCI CXX23)
set(CONFIG_CONTENT "// Auto-generated configuration header\n#ifndef CONFIG_H\n#define CONFIG_H\n\n")

foreach(OPT ${OPTIONS})
    if(NOT ${OPT})
        string(APPEND CONFIG_CONTENT "#define DISABLE_${OPT}\n")
        message(STATUS "Option ${OPT} disabled")
    endif()
endforeach()

string(APPEND CONFIG_CONTENT "#ifndef VERSION\n#define VERSION \"${VERSION_STRING}\"\n#endif\n")
string(APPEND CONFIG_CONTENT "\n#endif // CONFIG_H\n")
file(WRITE ${CMAKE_BINARY_DIR}/config.h "${CONFIG_CONTENT}")

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

# NetCeiver XML2 flags
if(NETCVCLIENT)
    execute_process(
        COMMAND xml2-config --cflags
        OUTPUT_VARIABLE XML2_CFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE XML2_CONFIG_RESULT
    )
    if(XML2_CONFIG_RESULT EQUAL 0 AND XML2_CFLAGS)
        separate_arguments(XML2_CFLAGS_LIST UNIX_COMMAND "${XML2_CFLAGS}")
        add_compile_options(${XML2_CFLAGS_LIST})
    endif()
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
)

if(DVBCSA)
    list(APPEND MINISATIP_SOURCES src/csa.cpp)
    list(APPEND MINISATIP_LIBS dvbcsa)
endif()
if(DVBCA)
    list(APPEND MINISATIP_SOURCES src/ca.cpp src/aes.cpp)
    list(APPEND MINISATIP_LIBS crypto dl)
endif()
if(NETCVCLIENT)
    list(APPEND MINISATIP_SOURCES src/netceiver.cpp)
    list(APPEND MINISATIP_LIBS netceiver xml2 lzma z)
endif()
if(DDCI)
    list(APPEND MINISATIP_SOURCES src/ddci.cpp)
endif()
if(NOT APPLE)
    list(APPEND MINISATIP_LIBS rt)
endif()

# Main library
add_library(minisatip_lib OBJECT ${MINISATIP_SOURCES})
target_include_directories(minisatip_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)
target_compile_options(minisatip_lib PRIVATE -include ${CMAKE_BINARY_DIR}/config.h)
if(STDCPPEXP_LIBRARY)
    target_link_libraries(minisatip_lib PRIVATE ${STDCPPEXP_LIBRARY})
endif()

# Main executable
add_executable(minisatip $<TARGET_OBJECTS:minisatip_lib>)
target_include_directories(minisatip PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)
set_target_properties(minisatip PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(STATIC)
    set(STATIC_LIBS "")
    foreach(LIB ${MINISATIP_LIBS})
        list(APPEND STATIC_LIBS "-l${LIB}.a")
    endforeach()
    target_link_libraries(minisatip PRIVATE ${STATIC_LIBS})
    target_link_options(minisatip PRIVATE -static)
else()
    target_link_libraries(minisatip PRIVATE ${MINISATIP_LIBS})
endif()

install(TARGETS minisatip RUNTIME DESTINATION bin)

# Testing
if(BUILD_TESTING)
    enable_testing()

    set(TEST_LIBS
        crypto
        dl
        pthread
    )
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
        tests/test_pmt.cpp
        tests/test_ddci.cpp
        tests/test_dvb.cpp
        tests/test_minisatip.cpp
        tests/test_utils.cpp
        tests/test_socketworks.cpp
        tests/utils/test_utils_dir.cpp
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
    target_compile_options(test_main_lib PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
        -fsanitize=leak
    )

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
        target_compile_options(${TEST_NAME} PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
            -fsanitize=leak
        )
        target_link_options(${TEST_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=leak
        )
        target_link_libraries(${TEST_NAME} PRIVATE ${TEST_LIBS})
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
endif()

message(STATUS "Version: ${VERSION_STRING}, FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_RELEASE}")
