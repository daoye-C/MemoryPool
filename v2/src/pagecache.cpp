// pagecache 申请内存 并 内存颗粒打碎！

#include "../include/pagecache.h"
#include <sys/mman.h>  // 这是unix linux上的库文件，到时候在docker上去调试
#include <cstring>


namespace MemoryPool
{
    // 按照页数组织内存 先在freespan找 不行再申请
    void* pagecache::pagealloc(size_t pgnum) // 分配 指定页数
    {
        std::lock_guard<std::mutex> lock(pagemutex);

        auto it = freespans.lower_bound(pgnum); // 这里lower_bound返回的是一个迭代器
        if(it != freespans.end())
        {
            Span* span = it->second;

            if(span->next)
            {
                freespans[it->first] = span->next;
            }
            else{
                freespans.erase(it);
            }

            // 切分
            if(span->pgnum > pgnum)
            {
                Span* newspan = new Span;
                newspan->pgaddr = static_cast<char*>(span->pgaddr) + pgnum * PGSIZE;
                newspan->pgnum = span->pgnum - pgnum;
                newspan->next = nullptr;

                auto& list = freespans[newspan->pgnum];
                newspan->next = list;
                list = newspan;

                span->pgnum = pgnum; // 正在使用，所以无需放进freespan中去
            }

            spanmap[span->pgaddr] = span;
            return span->pgaddr;

        }

        void* mem = systemalloc(pgnum);
        if(!mem) return nullptr;

        Span* span = new Span;
        span->pgaddr = mem;
        span->pgnum = pgnum;
        span->next = nullptr;
        
        spanmap[span->pgaddr] = span;
        return span->pgaddr;
    }

    void pagecache::pagedealloc(void* ptr, size_t pgnum)
    {
        std::lock_guard<std::mutex> lock(pagemutex);
        auto it = spanmap.find(ptr);
        if(it == spanmap.end()) return;

        Span* span = it->second;

        void* nextaddr = static_cast<char*>(span->pgaddr) + pgnum * PGSIZE;
        auto nextit = spanmap.find(nextaddr);
        
        if(nextit != spanmap.end())
        {
            Span* nextspan = nextit->second;

            bool found = false;
            auto& nextlist = freespans[nextspan->pgnum];
            if(nextlist == nextspan)
            {
                nextlist = nextlist->next;
                found = true;
            }
            else if(nextlist)
            {
                Span* prespan = nextlist;
                while(prespan->next)
                {
                    if(prespan->next == nextspan)
                    {
                        prespan->next = nextspan->next;
                        found = true;
                        break;
                    }
                    prespan = prespan->next;
                }
            }

            if(found)
            {
                span->pgnum += nextspan->pgnum;
                spanmap.erase(nextaddr);
                delete nextspan;
            }

            auto& list = freespans[span->pgnum];
            span->next = list;
            list = span;
        }
    }

    void* pagecache::systemalloc(size_t pgnum)
    {
        size_t size = pgnum * PGSIZE;

        void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS , -1, 0);

        if(ptr == MAP_FAILED) return nullptr;

        memset(ptr, 0, size);

        return ptr;
    }
}