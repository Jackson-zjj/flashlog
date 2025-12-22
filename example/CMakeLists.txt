cmake_minimum_required(VERSION 3.16)
project(logger_example)

add_executable(logger_example logger_example.cc)
target_link_libraries(logger_example logger)

add_executable(zstd_example zstd_example.cc)
target_link_libraries(zstd_example logger)

add_executable(crypto_example crypto_example.cc)
target_link_libraries(crypto_example logger)