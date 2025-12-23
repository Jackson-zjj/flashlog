include_guard(GLOBAL)

find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(zstd REQUIRED)
find_package(cryptopp REQUIRED)
find_package(Protobuf REQUIRED)

# include_directories(. third_party/cryptopp_cmake third_party/protobuf third_party/fmt/include)

# add_subdirectory(third_party/cryptopp_cmake)
# add_subdirectory(third_party/protobuf)
# add_subdirectory(third_party/zstd/build/cmake zstd)
# add_subdirectory(third_party/zlib)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(UTIL_SRCS utils/file_util.cc utils/sys_util.cc utils/sys_util_linux.cc)
    set(MMAP_SRCS mmap/mmap_aux.cc mmap/mmap_aux_linux.cc)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(UTIL_SRCS utils/file_util.cc utils/sys_util.cc utils/sys_util_win.cc)
    set(MMAP_SRCS mmap/mmap_aux.cc mmap/mmap_aux_win.cc)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin") # mac
    set(UTIL_SRCS utils/file_util.cc utils/sys_util.cc utils/sys_util_mac.cc)
    set(MMAP_SRCS mmap/mmap_aux.cc mmap/mmap_aux_linux.cc)
else()
    message(FATAL_ERROR "unsupport") 
endif()

file(GLOB FORMATTER_SCRS formatter/*.cc)
file(GLOB SINK_SRCS sinks/*.cc)
file(GLOB HANDLER_SRCS handler/*.cc)
file(GLOB TASK_SRCS scheduler/*.cc)
file(GLOB COMPRESS_SRCS compress/*.cc)
file(GLOB CRYPTO_SRCS crypto/*.cc)
file(GLOB PROTO_SRCS ${CMAKE_BINARY_DIR}/*.cc)

set(SRCS 
    ${UTIL_SRCS}
    ${MMAP_SRCS}
    ${FORMATTER_SCRS}
    ${SINK_SRCS}
    ${HANDLER_SRCS}
    ${TASK_SRCS}
    ${COMPRESS_SRCS}
    ${CRYPTO_SRCS}
    ${PROTO_SRCS}
)

add_library(logger STATIC ${SRCS})

target_link_libraries(logger PUBLIC
    fmt::fmt
    ZLIB::ZLIB
    zstd::libzstd_static
    cryptopp::cryptopp
    protobuf::libprotobuf
)