#include "utils/sys_util.h"

#include <window.h>

namespace logger {

size_t GetPageSize() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}

}   // namespace logger