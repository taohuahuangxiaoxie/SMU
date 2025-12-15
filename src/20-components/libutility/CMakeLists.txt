cmake_minimum_required(VERSION 3.10)

project(libutility)

set(CMAKE_CXX_STANDARD 14)

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

# 使用通用配置中的路径（如果已定义，否则使用相对路径）
if(DEFINED COMMON_INCLUDE_DIR)
    set(INCLUDE_DIR ${COMMON_INCLUDE_DIR})
    set(LIB_DIR ${COMMON_LIB_DIR})
else()
    # 兼容独立编译的情况
    set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../10-include)
    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib)
endif()

#根据CMAKE_SYSTEM_PROCESSOR来设置LIB_DIR
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(LIB_DIR ${LIB_DIR}/arm64)
else()
    set(LIB_DIR ${LIB_DIR}/amd64)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDE_DIR}
)

#生成共享库文件
add_library(
    ${PROJECT_NAME} SHARED
    src/Compression.cpp
    src/Version.cpp
)

# 设置输出库名称为 libutility.so，并设置输出目录为 LIB_DIR
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "utility"
    PREFIX "lib"
    LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR}
)

#链接依赖库 - 静态链接zstd到共享库中，使调用者无需链接zstd
# LIB_DIR already points to lib/${ARCH}, so use it directly
# 必须使用静态库，以便将zstd代码打包到libutility.so中
if(EXISTS ${LIB_DIR}/libzstd.a)
    # 使用-Wl,--whole-archive确保静态库的所有符号都被链接
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_link_libraries(${PROJECT_NAME} PRIVATE
            -Wl,--whole-archive
            ${LIB_DIR}/libzstd.a
            -Wl,--no-whole-archive
        )
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB_DIR}/libzstd.a)
    endif()
    # 静态库需要链接 pthread
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
else()
    message(FATAL_ERROR "找不到 zstd 静态库文件 (libzstd.a)，查找路径: ${LIB_DIR}")
endif()

