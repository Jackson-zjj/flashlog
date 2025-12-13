#pragma once

#include "log_handle.h"

namespace logger {
    
class VariadicLogHandle : public LogHandle {
public:
    // 继承构造函数
    using LogHandle::LogHandle;

    template <typename... Args>
    void Log(LogLevel lvl, LogSourceLoc loc, fmt::format_string<Args...> fmt, Args&&... args) {
        Log_(lvl, loc, fmt, std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    void Log(LogLevel lvl, fmt::format_string<Args...> fmt, Args&&... args) {
        Log_(lvl, LogSourceLoc{}, fmt, std::forward<Args>(args)...);
    }

private:
    template <typename... Args>
    void Log_(LogLevel lvl, LogSourceLoc& loc, fmt::basic_string_view<char> fmt, Args&&... args){
        if (!ShouldLog(lvl)) {
            return;
        }
        fmt::basic_memory_buffer<char, 256> buf;
        fmt::detail::vformat_to(buf, fmt, fmt::make_format_args(std::forward<Args>(args)...));
        LogMsg msg(lvl, loc, StringView(buf.data(), buf.size()));
        LogHandle::Log_(msg);
    }

};  // class VariadicLogHandle

}   // namespace logger
