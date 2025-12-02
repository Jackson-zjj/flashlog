cmake_minimum_required(VERSION 3.10)
project(flashlog)

set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)  # 强制使用 Debug 模式
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")  # -g 生成调试信息，-O0 关闭优化（避免断点错位）

set(INCLUDES . ${CMAKE_BINARY_DIR} logger)
include_directories(${INCLUDES})

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
add_executable(flashlog ${SRC_LIST})

add_subdirectory(logger)
target_link_libraries(flashlog PRIVATE logger)
find_package(GTest REQUIRED)
target_link_libraries(flashlog PRIVATE 
    GTest::gtest
    GTest::gtest_main
)