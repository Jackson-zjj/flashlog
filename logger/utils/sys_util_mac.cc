#include "utils/sys_util.h"

#include <unistd.h>
#include <sys/syscall.h>

namespace logger {

size_t GetPageSize() {
    return getpagesize();
}

size_t GetProcessId() {
  return static_cast<size_t>(::getpid());
}

size_t GetThreadId() {
    return static_cast<size_t>(syscall(SYS_thread_selfid));
}

}   // namespace logger