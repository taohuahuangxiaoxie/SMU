cmake_minimum_required(VERSION 3.10)

project(wcdb_test)

set(CMAKE_CXX_STANDARD 14)

# 抑制 WCDB 库相关的警告
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wno-pragmas)
    add_compile_options(-Wno-error=format-security)
    add_compile_options(-Wno-format)
endif()

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
add_compile_definitions(_WCDB_ENABLED_=1)

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
    ${PROJECT_NAME} "wcdb_test.cpp"
)

# 将 WCDB 目录标记为系统头文件目录以抑制警告
target_include_directories(${PROJECT_NAME} PRIVATE
    SYSTEM ${INCLUDE_DIR}/WCDB
)

#链接依赖库
find_package(Threads REQUIRED)
# LIB_DIR already points to lib/${ARCH}, so use it directly
target_link_libraries(${PROJECT_NAME} ${LIB_DIR}/libwcdb.so)