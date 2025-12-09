#pragma once

#ifdef DEBUGMODE
// 内部
#define LOG_DEBUG(...) ::logger::internal::log_impl("Debug", __FILE__, __LINE__, __VA_ARGS__)

#define LOG_INFO(...) ::logger::internal::log_impl("Info", __FILE__, __LINE__, __VA_ARGS__)

#define LOG_WARNING(...) ::logger::internal::log_impl("Warning", __FILE__, __LINE__, __VA_ARGS__)

#define LOG_ERROR(...) ::logger::internal::log_impl("Error", __FILE__, __LINE__, __VA_ARGS__)

#else
// 外部
#define LOG_DEBUG(...)

#define LOG_INFO(...)

#define LOG_WARNING(...)

#define LOG_ERROR(...)

#endif

namespace logger::internal {
    void log_impl(const char* flag, const char* file, int line, const char* format_str, ...);
}