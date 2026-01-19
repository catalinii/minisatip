# Compute derived feature flags
set(DDCI OFF)

if(DVBCA AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DDCI ON)
endif()

# Generate config.h by iterating over all options
set(OPTIONS DVBCSA DVBCA NETCVCLIENT LINUXDVB DDCI CXX23)
set(CONFIG_CONTENT "// Auto-generated configuration header\n#ifndef CONFIG_H\n#define CONFIG_H\n\n")

foreach(OPT ${OPTIONS})
    if(NOT ${OPT})
        string(APPEND CONFIG_CONTENT "#define DISABLE_${OPT}\n")
	message(STATUS "Option ${OPT} disabled")
    endif()
endforeach()

string(APPEND CONFIG_CONTENT "#define VERSION \"${VERSION_STRING}\"\n")
string(APPEND CONFIG_CONTENT "\n#endif // CONFIG_H\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.h "${CONFIG_CONTENT}")

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

# Force include config.h in all source files
add_compile_options(-include ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Core source and library lists
set(MINISATIP_SOURCES
    minisatip.cpp
    socketworks.cpp
    stream.cpp
    adapter.cpp
    httpc.cpp
    opts.cpp
    utils.cpp
    api/symbols.cpp
    api/variables.cpp
    utils/dvb/dvb_support.cpp
    utils/logging/logging.cpp
    utils/fifo.cpp
    utils/ticks.cpp
    utils/uuid.cpp
    dvb.cpp
    pmt.cpp
    tables.cpp
    dvbapi.cpp
    satipc.cpp
)
set(MINISATIP_LIBS pthread)

# Optional features - sources and libraries
if(DVBCSA)
    list(APPEND MINISATIP_SOURCES csa.cpp)
    list(APPEND MINISATIP_LIBS dvbcsa)
endif()
if(DVBCA)
    list(APPEND MINISATIP_SOURCES ca.cpp aes.cpp)
    list(APPEND MINISATIP_LIBS crypto dl)
endif()
if(NETCVCLIENT)
    list(APPEND MINISATIP_SOURCES netceiver.cpp)
    list(APPEND MINISATIP_LIBS netceiver xml2 lzma z)
endif()
if(DDCI)
    list(APPEND MINISATIP_SOURCES ddci.cpp)
endif()
if(NOT APPLE)
    list(APPEND MINISATIP_LIBS rt)
endif()

add_library(minisatip_lib OBJECT ${MINISATIP_SOURCES})

target_include_directories(minisatip_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Create the executable
add_executable(minisatip $<TARGET_OBJECTS:minisatip_lib>)
target_include_directories(minisatip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
set_target_properties(minisatip PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Link libraries
if(STATIC)
    # For static linking, use -l:libfoo.a syntax
    set(STATIC_LIBS "")
    foreach(LIB ${MINISATIP_LIBS})
        list(APPEND STATIC_LIBS "-l:lib${LIB}.a")
    endforeach()
    target_link_libraries(minisatip PRIVATE ${STATIC_LIBS})
    target_link_options(minisatip PRIVATE -static)
else()
    target_link_libraries(minisatip PRIVATE ${MINISATIP_LIBS})
endif()

# Install target
install(TARGETS minisatip RUNTIME DESTINATION bin)
