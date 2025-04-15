// 增加复用，碎化内存
#include "../include/centralcache.h"
#include "../include/pagecache.h"
#include <cassert>
#include <thread>


namespace MemoryPool
{
    static size_t Span_PG = 8;
    void* CentralCache::central_alloc(size_t index, size_t batchnum)
    {

        if(index >= FREE_LIST_SIZE || batchnum == 0)
        {
            return nullptr;
        }

        while(locks[index].test_and_set(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }

        void* result = nullptr;
        
        try
        {
            result = central_freelist[index].load(std::memory_order_relaxed);
            if(!result)
            {
                size_t size = (index + 1) * ALIGNMENT;
                result = fetchfrompagealloc(size);

                if(!result)
                {
                    locks[index].clear(std::memory_order_release);
                    return nullptr;
                }

                // 碎化内存块

                char* start = static_cast<char*>(result);
                size_t pgnum = (size + pagecache::PGSIZE -1 ) / pagecache::PGSIZE;
                size_t blocknum = 0;
                if(size <= Span_PG*pagecache::PGSIZE)
                {
                    blocknum = (Span_PG * pagecache::PGSIZE) / size;
                }
                else{
                    blocknum = (pgnum * pagecache::PGSIZE) / size;
                }
                
                size_t allocnum = std::min(batchnum, blocknum);

                if(allocnum > 1)
                {
                    for(size_t i = 1; i < allocnum; i++)
                    {
                        void* current = start + (i -1)*size;
                        void* next = start + i*size;
                        *reinterpret_cast<void**>(current) = next;
                    }
                    *reinterpret_cast<void**>(start + (allocnum - 1)*size) = nullptr;
                    
                }
                if(blocknum > allocnum)
                {
                    void* remainstart = start + allocnum*size ; 
                    for(size_t i = allocnum + 1; i < blocknum; i++)
                    {
                        void* current = start + (i-1)*size;
                        void* next = start + i*size;
                        *reinterpret_cast<void**>(current) = next;
                    }
                    *reinterpret_cast<void**>(start + (blocknum - 1)*size) = nullptr;

                    central_freelist[index].store(remainstart, std::memory_order_release);
                }
                

            }
            else{
                void* current = result;
                void* prev = nullptr;
                size_t count = 0;
                while(current && count < batchnum)
                {
                    prev = current;
                    current = *reinterpret_cast<void**>(current);
                    count ++;
                }

                if(prev)  // 防止一个块都没有
                {
                    *reinterpret_cast<void**>(prev) = nullptr;
                }
                central_freelist[index].store(current, std::memory_order_release);
            }
        }
        catch(...)
        {
            locks[index].clear(std::memory_order_release);
            throw;
        }
        locks[index].clear(std::memory_order_release);

        return result;
    }



    void CentralCache::central_dealloc(void* ptr, size_t num, size_t index)
    {
        if(!ptr || index >= FREE_LIST_SIZE)
            return;
        while(locks[index].test_and_set(std::memory_order_acquire))
            std::this_thread::yield();
        try
        {
            void* end = ptr;
            size_t count =1;
            while(*reinterpret_cast<void**>(end) != nullptr && count < num) // 逻辑上有问题
            {
                end = *reinterpret_cast<void**>(end);
                count ++;
            }
            
            void* current = central_freelist[index].load(std::memory_order_relaxed);
            *reinterpret_cast<void**>(end) = current;
            central_freelist[index].store(ptr, std::memory_order_release);

        }
        catch(...)
        {
            locks[index].clear(std::memory_order_release);
            throw;
        }

        locks[index].clear(std::memory_order_release);
    }


    void* CentralCache::fetchfrompagealloc(size_t size)
    {
        // 向上取整
        size_t pgnum = (size + pagecache::PGSIZE - 1) / pagecache::PGSIZE;

        if(size <= Span_PG * pagecache::PGSIZE)
        {
            return pagecache::getinstance().pagealloc(Span_PG);
        }
        else
        {
            return pagecache::getinstance().pagealloc(pgnum);
        }
    }
}