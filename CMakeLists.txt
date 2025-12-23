cmake_minimum_required(VERSION 3.10)
project(flashlog)

set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)  # 强制使用 Debug 模式
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")  # -g 生成调试信息，-O0 关闭优化（避免断点错位）

add_compile_definitions(DEBUGMODE=1)

find_package(GTest REQUIRED)

set(INCLUDES . ${CMAKE_BINARY_DIR} logger)
include_directories(${INCLUDES})

add_subdirectory(logger)
add_subdirectory(example)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
add_executable(flashlog ${SRC_LIST})

target_link_libraries(flashlog PRIVATE logger)
target_link_libraries(flashlog PRIVATE GTest::gtest_main)