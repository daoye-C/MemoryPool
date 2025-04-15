#pragma once
#include "common.h"
#include <mutex>


namespace MemoryPool
{
    class CentralCache
    {
    public:
        static CentralCache& getinstance()
        {
            static CentralCache instance;
            return instance;
        }

        void* central_alloc(size_t index);
        void  central_dealloc(void* ptr, size_t num, size_t index);

    private:
        
        std::array<std::atomic<void*>, FREE_LIST_SIZE> central_freelist;
        std::array<std::atomic_flag, FREE_LIST_SIZE> locks;

    private:
        
        void* fetchfrompagealloc(size_t size);
        // void returntopagealloc(void* ptr, size_t size);

        CentralCache()
        {
            for(auto& ptr : central_freelist)
            {
                ptr.store(nullptr, std::memory_order_relaxed);
            }
            for(auto& lock : locks)
            {
                lock.clear();
            }
        }

    };
}