# 通用CMake配置
# 支持架构选择和路径配置

# 默认架构为amd64
if(NOT DEFINED ARCH)
    set(ARCH "amd64" CACHE STRING "Target architecture")
endif()

# 验证架构参数
if(NOT ARCH MATCHES "^(amd64|arm64)$")
    message(FATAL_ERROR "Invalid ARCH: ${ARCH}. Must be 'amd64' or 'arm64'")
endif()

message(STATUS "Building for architecture: ${ARCH}")

# 设置库目录路径
# common.cmake是从顶层CMakeLists.txt中include的，所以CMAKE_CURRENT_SOURCE_DIR就是项目根目录
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${ROOT_DIR}/src/10-include)

# 根据架构使用对应的库目录
set(LIB_DIR ${ROOT_DIR}/lib/${ARCH})

# 加载交叉编译工具链（仅arm64需要）
if(ARCH STREQUAL "arm64")
    if(EXISTS ${ROOT_DIR}/tool/cmake/toolchain_arm64.cmake)
        set(CMAKE_TOOLCHAIN_FILE ${ROOT_DIR}/tool/cmake/toolchain_arm64.cmake)
        message(STATUS "Using cross-compilation toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    else()
        message(WARNING "ARM64 toolchain file not found, using default compiler")
    endif()
endif()

message(STATUS "Library directory: ${LIB_DIR}")
message(STATUS "Include directory: ${INCLUDE_DIR}")

# 设置构建类型（如果没有指定）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
endif()

# 根据构建类型设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
    message(STATUS "Build type: Debug")
else()
    add_compile_options(-O3 -DNDEBUG)
    message(STATUS "Build type: Release")
endif()

# 设置构建输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin-${ARCH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib-${ARCH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib-${ARCH})

# 导出变量供子项目使用
set(COMMON_INCLUDE_DIR ${INCLUDE_DIR})
set(COMMON_LIB_DIR ${LIB_DIR})
set(COMMON_ARCH ${ARCH})

