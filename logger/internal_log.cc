#include <fmt/chrono.h>
#include <fmt/core.h>

namespace logger::internal {
    void log_impl(const char* flag, char* file, int line, const char* format_str, ...) {
        va_list args;
        va_start(args, format_str);
        std::string log_msg = fmt::vformat(format_str, fmt::make_format_args(args));
        va_end(args);
        fmt::print("[{}] [{}:{}] [{}] {} \n", flag, file, line, std::chrono::system_clock::now(), log_msg);
    }

} // namespace logger::internal