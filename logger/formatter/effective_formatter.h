#pragma once

#include "formatter.h"

namespace logger {

/// @brief 高效格式化器：格式在解析日志时自定义
class EffectiveFormatter final : public Formatter {
public:
    ~EffectiveFormatter() override = default;

    void Format(const LogMsg& msg, MemoryBuf* buf);

};  // class EffectiveFormatter

} // namespace logger
