cmake_minimum_required(VERSION 3.10)

project(zstd_test)

set(CMAKE_CXX_STANDARD 14)

# 抑制警告
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wno-pragmas)
    add_compile_options(-Wno-error=format-security)
    add_compile_options(-Wno-format)
endif()

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

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDE_DIR}
)

#生成目标文件
add_executable(
    ${PROJECT_NAME} "zstd_test.cpp"
)

#链接依赖库
# 优先使用 .so 文件，如果不存在则使用 .a 文件
if(EXISTS ${LIB_DIR}/libzstd.so.1.5.7)
    target_link_libraries(${PROJECT_NAME} ${LIB_DIR}/libzstd.so.1.5.7)
elseif(EXISTS ${LIB_DIR}/libzstd.so)
    target_link_libraries(${PROJECT_NAME} ${LIB_DIR}/libzstd.so)
elseif(EXISTS ${LIB_DIR}/libzstd.a)
    target_link_libraries(${PROJECT_NAME} ${LIB_DIR}/libzstd.a)
else()
    message(FATAL_ERROR "找不到 zstd 库文件，查找路径: ${LIB_DIR}")
endif()

# 如果是静态库，需要链接 pthread
if(EXISTS ${LIB_DIR}/libzstd.a)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif()

