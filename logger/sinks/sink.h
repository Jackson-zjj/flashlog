#pragma once

#include "log_msg.h"
#include "formatter/formatter.h"

namespace logger {

/// @brief 日志接收器
class LogSink {
public:
    LogSink() = default;

    LogSink(const LogSink& other) = delete;

    LogSink& operator=(const LogSink& other) = delete;

    virtual ~LogSink() = default;

    virtual void Log(const LogMsg&) = 0;

    virtual void SetFormatter(std::unique_ptr<Formatter> formatter) = 0;

    virtual void Flush() {}

};  // class sink

}   // namespace logger