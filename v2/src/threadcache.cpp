
#include "../include/threadcache.h"
#include "../include/centralcache.h"
#include "../include/common.h"

namespace MemoryPool{

void*
ThreadCache::allocate(size_t size)
{
    if(size == 0)
        size = ALIGNMENT;
    
    if(size > MAX_BYTES)
        return malloc(size);
    
    size_t index = SizeClass::getindex(size);


    if(void* ptr = freelist_[index])
    {
        freelistsize[index] --;
        freelist_[index] = *reinterpret_cast<void**>(ptr);
        return ptr;
    }
    return fetchfromCentralCache(index);
}

void
ThreadCache::deallocate(void* ptr, size_t size)
{
    if(size > MAX_BYTES)
    {
        free(ptr);
        return;
    }
    size_t index = SizeClass::getindex(size);

    // 头插法
    *reinterpret_cast<void**>(ptr) = freelist_[index];
    freelist_[index] = ptr;

    freelistsize[index] ++;

    if(shouldReturnCentralCache(index))
    {
        returntoCentralCache(freelist_[index], size);
    }
    
}

bool 
ThreadCache::shouldReturnCentralCache(size_t index)
{
    size_t MAX_tfreelist = 64;  // 内存大小设置
    return freelistsize[index] > MAX_tfreelist;
}




void* 
ThreadCache::fetchfromCentralCache(size_t index)
{
    void* start = CentralCache::getinstance().central_alloc(index);
    if(!start) return nullptr;

    void* result = start;
    freelist_[index] = *reinterpret_cast<void**>(start);

    size_t num = 0;
    void* current = start;

    while(current != nullptr)
    {
        num ++;
        current = *reinterpret_cast<void**>(current);
    }

    freelistsize[index] += num;
    return result;
}

void
ThreadCache::returntoCentralCache(void* start, size_t size)
{
    size_t index = SizeClass::getindex(size);

    size_t alignedsize = SizeClass::roundup(size);

    size_t batchnum = freelistsize[index];
    if(batchnum <= 1) return ;

    size_t keepnum = std::max(batchnum / 4 , size_t(1));
    size_t returnnum = keepnum - batchnum;

    char* current = static_cast<char*>(start);
    char* splitnode = current;

    for(size_t i = 0 ; i < keepnum -1; i++)
    {
        splitnode = reinterpret_cast<char*>(*reinterpret_cast<void**>(splitnode));
        if(splitnode == nullptr)
        {
            returnnum = batchnum - (i + 1);
            break;
        }
    }

    if(splitnode != nullptr)
    {
        void* nextnode = *reinterpret_cast<void**>(splitnode);
        *reinterpret_cast<void**>(splitnode) = nullptr;

        freelist_[index] = start;
        freelistsize[index] = keepnum;

        if(returnnum > 0 && nextnode != nullptr)
        {
            CentralCache::getinstance().central_dealloc(nextnode, returnnum * alignedsize, index);
        }
    }
}

}