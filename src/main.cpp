#include <iostream>
#include <string.h>

#include "utils/sys_util.h"
#include "mmap/mmap_aux.h"

const char* kFilePath = "test/log.txt";

int main() {
    logger::MMapAux obj(kFilePath);
    const char* str = "Hello, World! \n";
    obj.Push(str, strlen(str));
    uint8_t* data = obj.Data();
    std::cout << "content:" << data << std::endl;
    return 0;
}