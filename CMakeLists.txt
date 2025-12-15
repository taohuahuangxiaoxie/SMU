cmake_minimum_required(VERSION 3.10)

project(SMU_TESTS)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含通用配置
include(tool/cmake/common.cmake)

# 添加组件子目录
add_subdirectory(src/20-components/smu_utils)

# 添加测试项目子目录
add_subdirectory(src/21-test_demo/wcdb_test)
add_subdirectory(src/21-test_demo/zstd_test)

