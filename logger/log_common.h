#pragma once

#include <utility>
#include <string>

namespace logger {

using StringView = std::string_view;

enum class LogLevel {
    kInfo = 1,
    kDebug = 2,
    kError = 3
};

/// @brief 日志源
struct LogSourceLoc {
    /* data */
};


}// namespace logger