#include "memory_pool.h"

namespace memorypools
{
    MemoryPool::MemoryPool(size_t Blocksize): Blocksize_(Blocksize)
    {}

    MemoryPool::~MemoryPool()
    {
        Slot* cur = firstSlot_;
        
        while(cur)
        {
            Slot* next = cur->next;

            operator delete(reinterpret_cast<void*>(cur));
            cur = next;
        }

    }

    void MemoryPool::init(size_t size)
    {
        assert(size > 0);
        Slotsize_ = size;
        firstSlot_ = nullptr;
        lastSlot_ = nullptr;
        curSlot_ = nullptr;
        freelist_ = nullptr;
    }

    void* MemoryPool::allocate()  
    {
        if(freelist_ != nullptr)
        {
            {
                std::lock_guard<std::mutex> lock(mutexforfreelist_);
                if(freelist_ != nullptr)
                {
                    Slot* tmp = freelist_;
                    freelist_ = freelist_->next;
                    return tmp;
                }
            }
        }

        Slot* tmp;

        {
            std::lock_guard<std::mutex> lock (mutexforfreelist_);
            if(curSlot_ >= lastSlot_)
            {
                allocateNewBlock();
            }

            tmp = curSlot_;
            curSlot_ += Slotsize_ / sizeof(Slot); // 新增字节数

        }

        return tmp;
    }

    void MemoryPool::deallocate(void* ptr)  //不需要析构对象只需要回收内存
    {
        if(ptr)
        {
            std::lock_guard<std::mutex> lock (mutexforfreelist_);
            reinterpret_cast<Slot*>(ptr) -> next = freelist_;
            freelist_ = reinterpret_cast<Slot*>(ptr);
        }
    }

    void MemoryPool::allocateNewBlock()
    {
        void* newblock = operator new(Blocksize_);
        reinterpret_cast<Slot*>(newblock)->next = firstSlot_;
        firstSlot_ = reinterpret_cast<Slot*>(newblock);

        char* body = reinterpret_cast<char*>(newblock) + sizeof(Slot*);   // newblock 开始的位置存储了一个Slot*  就是next
        size_t paddingSize = paddpoint(body, Slotsize_); 
        curSlot_ = reinterpret_cast<Slot*>(body+paddingSize); // 对照allocate()可发现最后一个slot不会被使用！！

        lastSlot_ = reinterpret_cast<Slot*>(reinterpret_cast<size_t>(newblock) + Blocksize_ - Slotsize_ +1);  // 这是在找最后一个Slot

        freelist_ = nullptr;
    }

    // 计算内存对齐填充的字节数
    size_t MemoryPool::paddpoint(char* ptr, size_t align)
    {
        return (align - reinterpret_cast<size_t>(ptr)) % align;
    }


    void HashBucket::init_memorypool()
    {
        for(int i = 0; i < MEMORY_POOL_num; i++)
        {
            get_memorypool(i).init((i+1)*BASE_Slot_size);
        }
    }

    MemoryPool& HashBucket::get_memorypool(int index)
    {
        static MemoryPool memorypool[MEMORY_POOL_num];
        return memorypool[index];
    }


}