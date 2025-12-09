#pragma once

#include "log_common.h"

namespace logger {

/// @brief 日志信息
struct LogMsg {
    LogMsg(LogLevel lvl, LogSourceLoc loc, StringView msg) : level(lvl), location(loc), message(msg) {}

    LogMsg(LogLevel lvl, StringView msg) : LogMsg(lvl, LogSourceLoc{}, msg) {}

    LogMsg(const LogMsg& other) = default;

    LogMsg& operator=(const LogMsg& other) = default;

    LogLevel level;
    LogSourceLoc location;
    StringView message;
};

}// namespace logger