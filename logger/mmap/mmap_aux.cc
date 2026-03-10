#include "mmap/mmap_aux.h"

#include <string.h>
#include <iostream>

#include "utils/file_util.h"
#include "utils/sys_util.h"

namespace logger {

constexpr size_t kDafaultCapacity = 512 * 1024;  // 512KB

MMapAux::MMapAux(fpath file_path) : capacity_(0), size_(0), handle_(nullptr), file_path_(std::move(file_path)) {
    size_t size = fs::GetFileSize(file_path);
    size_t new_size = std::max(size, kDafaultCapacity);
    Reserve_(new_size);
    Init_();
}

void MMapAux::Init_() {
    MMapHeader* header = Header_();
    if (!header) {
        return;
    }
    if (header->magic != MMapHeader::kMagic) {
        header->magic = MMapHeader::kMagic;
        header->size = 0;
    }
}

void MMapAux::Push(const void* data, size_t size) {
    if (!isValid_()) {
        return;
    }
    size_t new_size = Size() + size;
    EnsureCapacity_(new_size);
    memcpy(Data() + Size(), data, size);
    Header_()->size = new_size;
}

void MMapAux::Resize(size_t new_size) {
    if (!isValid_()) {
        return;
    }
    EnsureCapacity_(new_size);
    Header_()->size = new_size;
}

void MMapAux::Clear() {
    if (!isValid_()) {
        return;
    }
    Header_()->size = 0;
}

size_t MMapAux::Size() const {
    if (!isValid_()) {
        return 0;
    }
    return Header_()->size;
}

bool MMapAux::Empty() const {
    return Size() == 0;
}

uint8_t* MMapAux::Data() const {
    if (!isValid_()) {
        return nullptr;
    }
    // uint8_t 原始字节流的通用指针
    return static_cast<uint8_t*>(handle_) + sizeof(MMapHeader);
}

double MMapAux::GetRatio() const {
    if (!isValid_()) {
        return 0.0;
    }
    return static_cast<double>(Size()) / (Capacity_() - sizeof(MMapHeader));
}

static size_t GetValidCapacity(size_t size) {
    // 向上取整
    size_t page_size = GetPageSize();
    return (size + page_size - 1) / page_size * page_size;
}

void MMapAux::Reserve_(size_t new_capacity) {
    size_t real_capacity = GetValidCapacity(new_capacity);
    if (real_capacity <= capacity_) {
        return;
    }
    Unmap_();
    // 内存映射
    TryMap_(real_capacity); // todo check mmap result
    capacity_ = real_capacity;
}

void MMapAux::EnsureCapacity_(size_t new_size) {
    size_t new_capacity = new_size + sizeof(MMapHeader);
    size_t real_capacity = GetValidCapacity(new_capacity);
    if (real_capacity <= capacity_) {
        return;
    }
    Reserve_(real_capacity);
}

MMapAux::MMapHeader* MMapAux::Header_() const {
    if (!handle_) {
        return nullptr;
    }
    return static_cast<MMapHeader*>(handle_);
}

size_t MMapAux::Capacity_() const {
    return capacity_;
}

bool MMapAux::isValid_() const {
    MMapHeader* header = Header_();
    if (!header) {
        return false;
    }
    return header->magic == MMapHeader::kMagic;
}

}   //namespace logger