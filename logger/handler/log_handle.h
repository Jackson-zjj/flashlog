#pragma once

#include <memory>
#include <initializer_list>
#include <vector>
#include <atomic>

#include "log_common.h"
#include "log_msg.h"

namespace logger {

class LogSink;
using LogSinkPtr = std::shared_ptr<logger::LogSink>;
using LogSinkPtrList = std::initializer_list<LogSinkPtr>;

/// @brief 日志处理器
class LogHandle {
public:
    // Construct
    explicit LogHandle(LogSinkPtr sink);

    explicit LogHandle(LogSinkPtrList sinks);

    template <typename It>
    LogHandle(It begin, It end) : sinks_(begin, end) {}
    
    ~LogHandle() = default;

    LogHandle(const LogHandle&) = delete;

    LogHandle& operator=(const LogHandle&) = delete;

    // public interface
    void SetLevel(LogLevel);

    LogLevel GetLevel() const;

    void Log(LogLevel, LogSourceLoc, StringView) const;

protected: 
    inline bool ShouldLog(LogLevel level) const {
        return level >= level_; // todo add msg empty check
    }

    void Log_(const LogMsg&) const;

private:
    std::vector<LogSinkPtr> sinks_;
    std::atomic<LogLevel> level_;

};  // class LogHandle

}   // namespace logger