#include "mmap/mmap_aux.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

namespace logger {

/// @brief linux下实现mmap
/// @param capacity 映射内存大小
/// @return success
bool MMapAux::TryMap_(size_t capacity) {
    int fd = open(file_path_.string().c_str(), O_RDWR | O_CREAT, S_IRWXU);
    if (fd == -1) {
        return false;
    }
    // 改变文件大小（系统调用函数）
    ftruncate(fd, capacity);

    handle_ = mmap(NULL, capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return handle_ != MAP_FAILED;
}

/// @brief 解除映射
void MMapAux::Unmap_() {
    if (handle_) {
        munmap(handle_, capacity_);
    }
    handle_ = nullptr;
}


}   //namespace logger