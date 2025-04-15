#pragma once
#include "threadcache.h"

namespace MemoryPool
{
    class mempool
    {
    public:
        static void* allocate(size_t size)
        {
            return ThreadCache::getinstance()->allocate(size);
        }

        static void deallocate(void* ptr, size_t size)
        {
            ThreadCache::getinstance()->deallocate(ptr, size);
        }
    };
}