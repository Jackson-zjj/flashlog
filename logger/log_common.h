#pragma once

#include <utility>
#include <string>

#include <fmt/format.h>

namespace logger {

using StringView = std::string_view;
using MemoryBuf = std::string;

#define LOGGER_LEVEL_TRACE 0
#define LOGGER_LEVEL_DEBUG 1
#define LOGGER_LEVEL_INFO 2
#define LOGGER_LEVEL_WARN 3
#define LOGGER_LEVEL_ERROR 4
#define LOGGER_LEVEL_CRITICAL 5
#define LOGGER_LEVEL_OFF 6

enum class LogLevel {
    kTrace = LOGGER_LEVEL_TRACE,
    kDebug = LOGGER_LEVEL_DEBUG,
    kInfo = LOGGER_LEVEL_INFO,
    kWarn = LOGGER_LEVEL_WARN,
    kError = LOGGER_LEVEL_ERROR,
    kFatal = LOGGER_LEVEL_CRITICAL,
    kOff = LOGGER_LEVEL_OFF 
};

#define LOGGER_ACTIVE_LEVEL LOGGER_LEVEL_TRACE

/// @brief 日志源
struct LogSourceLoc {
    LogSourceLoc() = default;

    LogSourceLoc(StringView file_name_in, uint32_t line_in, StringView func_name_in) 
        : file_name(file_name_in), line(line_in), func_name(func_name_in) {
        if (file_name.empty()) {
            return;
        }
        // 解析去除文件目录
        size_t pos = file_name.rfind('/');
        if (pos != StringView::npos) {
            file_name = file_name_in.substr(pos + 1);

        } else {
            pos = file_name.rfind("\\");
            if (pos != StringView::npos) {
                file_name = file_name_in.substr(pos + 1);
            }
        }

    }

    LogSourceLoc(const LogSourceLoc& other) = default;

    LogSourceLoc& operator=(const LogSourceLoc& other) = default;

    StringView file_name;
    uint32_t line{0};
    StringView func_name;
};


}// namespace logger