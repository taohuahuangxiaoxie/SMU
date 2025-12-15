cmake_minimum_required(VERSION 3.10)

project(SMU_TESTS)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含通用配置
include(tool/cmake/common.cmake)

# 自动添加 20-components 下的所有组件
file(GLOB COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/20-components/*")
foreach(COMPONENT_DIR ${COMPONENT_DIRS})
    if(IS_DIRECTORY ${COMPONENT_DIR})
        # 检查是否存在 CMakeLists.txt
        if(EXISTS "${COMPONENT_DIR}/CMakeLists.txt")
            get_filename_component(COMPONENT_NAME ${COMPONENT_DIR} NAME)
            message(STATUS "Adding component: ${COMPONENT_NAME}")
            add_subdirectory(${COMPONENT_DIR})
        endif()
    endif()
endforeach()

# 添加测试项目子目录
add_subdirectory(src/21-test_demo/wcdb_test)
add_subdirectory(src/21-test_demo/zstd_test)

