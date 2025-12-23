#include "effective_formatter.h"

#include <chrono>

#include "effective_msg.pb.h"
#include "utils/sys_util.h"

namespace logger {
    
void EffectiveFormatter::Format(const LogMsg& msg, MemoryBuf* buf) {
    EffectiveMsg formatter;
    formatter.set_level(static_cast<int>(msg.level));
    formatter.set_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count());
    formatter.set_pid(GetProcessId());
    formatter.set_tid(GetThreadId());
    formatter.set_line(msg.location.line);
    formatter.set_file_name(msg.location.file_name.data(), msg.location.file_name.size());
    formatter.set_func_name(msg.location.func_name.data(), msg.location.func_name.size());
    formatter.set_log_info(msg.message.data(), msg.message.size());

    size_t len = formatter.ByteSizeLong();
    buf->reserve(len);
    formatter.SerializeToArray(buf->data(), len);
}

} // namespace logger
