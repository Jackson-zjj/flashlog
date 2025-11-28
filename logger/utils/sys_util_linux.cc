#include "utils/sys_util.h"

#include <unistd.h>

namespace logger {

size_t GetPageSize() {
    return getpagesize();
}

}   // namespace logger