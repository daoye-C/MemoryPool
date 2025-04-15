#pragma once
#include <cstddef>
#include <array>
#include <atomic>



namespace MemoryPool
{
    constexpr const size_t ALIGNMENT = 8;  // 8 为步长
    constexpr const size_t MAX_BYTES = 256*1024;  // 256KB
    constexpr const size_t FREE_LIST_SIZE = MAX_BYTES / ALIGNMENT;  // 32K 种大小

    

    class SizeClass
    {
    public:
        static size_t roundup(size_t bytes)
        {
            return (bytes + ALIGNMENT - 1) & ~ (ALIGNMENT - 1);   
        }
        static size_t getindex(size_t size)
        {
            size = std::max(size, ALIGNMENT);

            return (size + ALIGNMENT -1) / ALIGNMENT -1; // 向上取整  再-1
        }
    };

    
}