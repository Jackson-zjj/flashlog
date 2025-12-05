#pragma once

#include "log_msg.h"

namespace logger {

/// @brief 日志接收器
class LogSink {
public:

    // public interact
    void Log(const LogMsg&);

private:
    

};  // class sink

}   // namespace logger