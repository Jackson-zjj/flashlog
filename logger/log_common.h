#pragma once

#include <utility>
#include <string>

namespace logger {

using StringView = std::string_view;

enum class LogLevel {
    kDebug,
    kInfo,
    kWarn,
    kError,
    kOff
};

/// @brief 日志源
struct LogSourceLoc {
    LogSourceLoc() = default;

    LogSourceLoc(StringView file_name_in, uint32_t line_in, StringView func_name_in) 
        : file_name(file_name_in), line(line_in), func_name(func_name_in) {
        if (file_name.empty()) {
            return;
        }
        // 解析去除文件目录
        size_t pos = file_name.rfind('/');
        if (pos != StringView::npos) {
            file_name = file_name_in.substr(pos + 1);

        } else {
            pos = file_name.rfind("\\");
            if (pos != StringView::npos) {
                file_name = file_name_in.substr(pos + 1);
            }
        }

    }

    LogSourceLoc(const LogSourceLoc& other) = default;

    LogSourceLoc& operator=(const LogSourceLoc& other) = default;

    StringView file_name;
    uint32_t line{0};
    StringView func_name;
};


}// namespace logger