#pragma once

#include "log_common.h"

namespace logger {

/// @brief 日志信息
struct LogMsg {
    LogMsg(LogLevel lvl, LogSourceLoc& loc, StringView& msg) : level(lvl), location(std::move(loc)), message(std::move(msg)) {}

    LogLevel level;
    LogSourceLoc location;
    StringView message;
};

}// namespace logger