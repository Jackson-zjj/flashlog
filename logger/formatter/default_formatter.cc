#include "default_formatter.h"

#include "fmt/chrono.h"
#include "fmt/core.h"

#include "utils/sys_util.h"

namespace logger {

void DefaultFormatter::Format(const LogMsg& msg, MemoryBuf* dest) {
    constexpr char kLogLevelStrp[] = "DIWEO";  // debug、info、warn、error、off
    auto now = std::chrono::system_clock::now();
    dest->reserve(256);
    fmt::format_to(std::back_inserter(*dest), "[{:%Y-%m-%d %H:%M:%S}] [{}] [{}:{}] [{}:{}] {}", 
        now, 
        kLogLevelStrp[static_cast<int>(msg.level)], 
        msg.location.file_name, msg.location.line, 
        GetProcessId(), GetThreadId(), 
        msg.message);
}
    
}   // namespace logger
