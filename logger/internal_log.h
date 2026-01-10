#pragma once

#include <fmt/chrono.h>
#include <fmt/core.h>

#ifdef DEBUGMODE
// 内部
#define DEBUG(...) \
    fmt::print("[DEBUG] [{}:{}] {} {}\n", __FILE__, __LINE__, std::chrono::system_clock::now(), fmt::format(__VA_ARGS__))

#define INFO(...) \
    fmt::print("[INFO] [{}:{}] {} {}\n", __FILE__, __LINE__, std::chrono::system_clock::now(), fmt::format(__VA_ARGS__))

#define WARNING(...) \
    fmt::print("[WARNING] [{}:{}] {} {}\n", __FILE__, __LINE__, std::chrono::system_clock::now(), fmt::format(__VA_ARGS__))

#define ERROR(...) \
    fmt::print("[ERROR] [{}:{}] {} {}\n", __FILE__, __LINE__, std::chrono::system_clock::now(), fmt::format(__VA_ARGS__))

#else
// 外部
#define DEBUG(...)

#define INFO(...)

#define WARNING(...)

#define ERROR(...)

#endif