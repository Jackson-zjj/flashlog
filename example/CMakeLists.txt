cmake_minimum_required(VERSION 3.16)
project(logger_example)

add_executable(logger_example logger_example.cc)
target_link_libraries(logger_example logger)