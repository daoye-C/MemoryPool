#pragma once
#include "common.h"
#include <mutex>
#include <map>


namespace MemoryPool
{
    class pagecache
    {
    public:
        static const size_t PGSIZE = 4096;  // 页面大小 4KB

        static pagecache& getinstance()
        {
            static pagecache instance;
            return instance;
        }


        void* pagealloc(size_t pgnum);
        void  pagedealloc(void* ptr, size_t pgsize);

    private:
        pagecache() = default;

        void* systemalloc(size_t pgnum);

        // void systemdealloc(void* ptr, size_t size);

    private:
        struct Span
        {
            void* pgaddr;
            size_t pgnum;
            struct Span* next;
        };

        std::map<size_t, Span*> freespans;
        std::map<void*, Span*> spanmap;

        std::mutex pagemutex;
        
    };
}