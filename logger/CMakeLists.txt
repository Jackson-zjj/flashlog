include_guard(GLOBAL)

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

set(FORMATTER_SCRS formatter/default_formatter.cc)
set(SINK_SRCS sinks/console_sink.cc)
set(LOGGER_SRCS handler/log_handle.cc)
set(TASK_SRCS scheduler/thread_pool.cc scheduler/scheduler.cc)

set(SRCS 
    ${UTIL_SRCS}
    ${MMAP_SRCS}
    ${FORMATTER_SCRS}
    ${SINK_SRCS}
    ${LOGGER_SRCS}
    ${TASK_SRCS}
)

add_library(logger STATIC ${SRCS})

find_package(fmt REQUIRED)
target_link_libraries(logger PUBLIC fmt::fmt)