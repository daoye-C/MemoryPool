#pragma once
#include "common.h"

namespace MemoryPool{

class ThreadCache
{
public:
    static ThreadCache* getinstance()
    {
        static thread_local ThreadCache instance;
        return &instance;
    }

    void* allocate(size_t size);
    void deallocate(void* ptr, size_t size);

private:
    // 与CentralCache 层对接
    void* fetchfromCentralCache(size_t index);   // 拿取
    void  returntoCentralCache(void* start, size_t index);  //放回

    bool shouldReturnCentralCache(size_t size);

private:

    std::array<void*, FREE_LIST_SIZE> freelist_;   // 元素类型  固定大小
    std::array<size_t, FREE_LIST_SIZE> freelistsize;
    
    ThreadCache()
    {
        freelist_.fill(nullptr);
        freelistsize.fill(0);
    }

};

}