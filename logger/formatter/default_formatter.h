#pragma once

#include "formatter.h"

namespace logger {
    
class DefaultFormatter : public Formatter {
public:
    void Format(const LogMsg& msg, MemoryBuf* buf) override;

};  // class DefaultFormatter

}   // namaspace logger