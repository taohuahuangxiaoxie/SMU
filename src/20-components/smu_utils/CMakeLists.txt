cmake_minimum_required(VERSION 3.10)

project(smu_utils)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use common config paths if defined, otherwise use relative paths
if(DEFINED COMMON_INCLUDE_DIR)
    set(INCLUDE_DIR ${COMMON_INCLUDE_DIR})
    set(LIB_DIR ${COMMON_LIB_DIR})
else()
    # Compatible with standalone build
    set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../10-include)
    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib)
endif()

# Include directories (headers are in 10-include directory)
include_directories(
    ${INCLUDE_DIR}
)

# Source files
set(SOURCES
    src/Version.cpp
    src/Compression.cpp
)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Set library output directory to top-level lib directory with architecture subdirectory
# Prefer ROOT_DIR from common config, otherwise calculate project root
if(DEFINED ROOT_DIR)
    set(PROJECT_ROOT_DIR "${ROOT_DIR}")
else()
    get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
endif()

# Get architecture (default to amd64 if not set)
if(DEFINED ARCH)
    set(TARGET_ARCH "${ARCH}")
else()
    set(TARGET_ARCH "amd64")
endif()

# Output to lib/${ARCH} directory
set(OUTPUT_LIB_DIR "${PROJECT_ROOT_DIR}/lib/${TARGET_ARCH}")
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LIB_DIR}"
)

# Set symbol visibility: hide all symbols, only export public API
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
    # Export public API symbols
    target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)
endif()

# Link zstd static library (users don't need to link zstd.so)
# Use architecture-specific zstd library if available, otherwise use common lib directory
if(EXISTS "${LIB_DIR}/${TARGET_ARCH}/libzstd.a")
    set(ZSTD_LIB_PATH "${LIB_DIR}/${TARGET_ARCH}/libzstd.a")
else()
    set(ZSTD_LIB_PATH "${LIB_DIR}/libzstd.a")
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${ZSTD_LIB_PATH})

# Set public header directories (headers are in 10-include directory, managed by common config)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Set compile options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
endif()

# Set DLL export on Windows
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SMU_UTILS_BUILD_DLL)
endif()

message(STATUS "smu_utils library will be output to: ${OUTPUT_LIB_DIR} (architecture: ${TARGET_ARCH})")
