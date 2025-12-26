#pragma once

#include "formatter.h"

namespace logger {
    
/// @brief 默认格式化器：[时间:年-月-日 时:分:秒] [日志等级] [消息来源] [进程ID:线程ID] 消息内容
class DefaultFormatter : public Formatter {
public:
    ~DefaultFormatter() override = default;

    void Format(const LogMsg& msg, MemoryBuf* buf) override;

};  // class DefaultFormatter

}   // namaspace logger