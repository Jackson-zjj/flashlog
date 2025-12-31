cmake_minimum_required(VERSION 3.10)
project(flashlog)

set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling AddressSanitizer in Debug mode")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

add_compile_definitions(DEBUGMODE=1)

find_package(GTest REQUIRED)

set(INCLUDES . ${CMAKE_BINARY_DIR} logger)
include_directories(${INCLUDES})

add_subdirectory(logger)
add_subdirectory(example)
add_subdirectory(decode)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
add_executable(flashlog ${SRC_LIST})

target_link_libraries(flashlog PRIVATE logger)
target_link_libraries(flashlog PRIVATE GTest::gtest_main)