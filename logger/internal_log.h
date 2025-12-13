#pragma once

#include <fmt/chrono.h>
#include <fmt/core.h>

#ifdef DEBUGMODE
// 内部
#define LOG_DEBUG(...) \
    fmt::print("[DEBUG] [{}:{}] {} {}\n", __FILE__, __LINE__, std::chrono::system_clock::now(), fmt::format(__VA_ARGS__))

#define LOG_INFO(...) \
    fmt::print("[INFO] [{}:{}] {} {}\n", __FILE__, __LINE__, std::chrono::system_clock::now(), fmt::format(__VA_ARGS__))

#define LOG_WARNING(...) \
    fmt::print("[WARNING] [{}:{}] {} {}\n", __FILE__, __LINE__, std::chrono::system_clock::now(), fmt::format(__VA_ARGS__))

#define LOG_ERROR(...) \
    fmt::print("[ERROR] [{}:{}] {} {}\n", __FILE__, __LINE__, std::chrono::system_clock::now(), fmt::format(__VA_ARGS__))

#else
// 外部
#define LOG_DEBUG(...)

#define LOG_INFO(...)

#define LOG_WARNING(...)

#define LOG_ERROR(...)

#endif