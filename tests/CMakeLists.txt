# Test configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Test compile flags with sanitizers
add_compile_options(
    -fsanitize=address
    -fno-omit-frame-pointer
    -fsanitize=leak
)

add_link_options(
    -fsanitize=address
    -fsanitize=leak
)

# Test-specific definitions
add_compile_definitions(
    ENIGMA
    TESTING
    DDCI_TEST
    DISABLE_STACKTRACE
    DISABLE_NETCVCLIENT
    VERSION="test"
)

# Build source list matching src/CMakeLists.txt
# Note: minisatip.cpp is included for global variable definitions (opts, version, etc.)
# The main() function from test files will be used instead
set(MAIN_SOURCES
    ${CMAKE_SOURCE_DIR}/src/minisatip.cpp
    ${CMAKE_SOURCE_DIR}/src/socketworks.cpp
    ${CMAKE_SOURCE_DIR}/src/stream.cpp
    ${CMAKE_SOURCE_DIR}/src/adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/httpc.cpp
    ${CMAKE_SOURCE_DIR}/src/opts.cpp
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/api/symbols.cpp
    ${CMAKE_SOURCE_DIR}/src/api/variables.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/dvb/dvb_support.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/logging/logging.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/fifo.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/ticks.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/uuid.cpp
    ${CMAKE_SOURCE_DIR}/src/dvb.cpp
    ${CMAKE_SOURCE_DIR}/src/pmt.cpp
    ${CMAKE_SOURCE_DIR}/src/tables.cpp
    ${CMAKE_SOURCE_DIR}/src/dvbapi.cpp
    ${CMAKE_SOURCE_DIR}/src/satipc.cpp
)

# Optional sources based on features
# Only include sources for which we have dependencies
set(TEST_LIBS
    crypto
    dl
    pthread
)

if(NOT APPLE)
    list(APPEND TEST_LIBS rt)
endif()

# Add dvbcsa if available
find_library(DVBCSA_LIB dvbcsa)
if(DVBCSA_LIB)
    list(APPEND MAIN_SOURCES ${CMAKE_SOURCE_DIR}/src/csa.cpp)
    list(APPEND TEST_LIBS dvbcsa)
else()
    add_compile_definitions(DISABLE_DVBCSA)
endif()

# Always include these for tests
list(APPEND MAIN_SOURCES
    ${CMAKE_SOURCE_DIR}/src/ca.cpp
    ${CMAKE_SOURCE_DIR}/src/aes.cpp
    ${CMAKE_SOURCE_DIR}/src/ddci.cpp
)

# Test source files
set(TEST_SOURCES
    test_adapter.cpp
    test_opts.cpp
    test_ca.cpp
    test_pmt.cpp
    test_ddci.cpp
    test_dvb.cpp
    test_minisatip.cpp
    test_utils.cpp
    test_socketworks.cpp
    utils/test_utils_dir.cpp
)

# Create test executables
foreach(TEST_SOURCE ${TEST_SOURCES})
    # Get test name from source file
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    # Create executable
    add_executable(${TEST_NAME} ${TEST_SOURCE} ${MAIN_SOURCES})
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}/src
    )
    target_link_libraries(${TEST_NAME} PRIVATE ${TEST_LIBS})

    # Register as CTest test
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
