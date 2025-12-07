#include "log_handle.h"

#include "sinks/sink.h"

namespace logger {

LogHandle::LogHandle(LogSinkPtr sink) : level_(LogLevel::kInfo) {
    sinks_.push_back(std::move(sink));
}

LogHandle::LogHandle(LogSinkPtrList sinks) : level_(LogLevel::kInfo) {
    for (auto& sink : sinks) {
        sinks_.push_back(std::move(sink));
    }
}

inline void LogHandle::SetLevel(LogLevel level) {
    level_ = level;
}

inline LogLevel LogHandle::GetLevel() const {
    return level_;
}

void LogHandle::Log(LogLevel level, LogSourceLoc& loc, StringView& msg_str) const {
    if (!ShouldLog(level)) {
        return;
    }
    LogMsg msg(level, loc, msg_str);
    Log_(msg);
}

inline bool LogHandle::ShouldLog(LogLevel level) const {
    return level >= level_; // todo add msg empty check
}

void LogHandle::Log_(const LogMsg& msg) const {
    for (auto& sink : sinks_) {
        sink->Log(msg);
    }
}


}   // namespace logger