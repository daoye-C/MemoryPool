#pragma once 

#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>


using namespace std;

namespace memorypools
{

#define MEMORY_POOL_num 64
#define BASE_Slot_size 8
#define MAX_Slot_size 512


struct Slot
{
    Slot* next;
};


class MemoryPool
{
public:
    MemoryPool(size_t Blocksize = 4096);
    ~MemoryPool();

    void init(size_t);

    void* allocate();           // 分配-先从freelist找 没有再去内存池找-内存不足申请分配-足够则直接返回
    void  deallocate(void*);    // 将指针放回freelist_

private:
    void allocateNewBlock();
    size_t paddpoint(char* p, size_t align);    // 找到对其内存所需填充的字节

private:

    int Blocksize_;
    int Slotsize_;

    // 管理内存池使用的指针
    Slot* firstSlot_;
    Slot* lastSlot_;

    // 分配内存池使用的指针
    Slot* curSlot_;
    Slot* freelist_;
    

    std::mutex mutexforfreelist_;    // 更新自由链表的锁
    std::mutex mutexforblock_;       // 更新内存池的锁

};



class HashBucket
{
public:
    static void init_memorypool();
    static MemoryPool& get_memorypool(int);


    static void* usememory(size_t size) // 分配内存
    {
        if(size <= 0)
            return nullptr;
        if(size > MAX_Slot_size)
            return operator new(size);
        
        return get_memorypool((size + 7) / BASE_Slot_size - 1).allocate();    // 因为索引是从0开始所以减一
    }


    static void freememory(void* p, size_t size)
    {
        if(!p)
            return ;
        if(size > MAX_Slot_size)
        {    
            operator delete(p);
            return ;
        }
        get_memorypool((size + 7) / BASE_Slot_size - 1).deallocate(p);
    }

    template<typename T, typename...Args>
    friend T* newElement(Args...arg);

    template<typename T>
    friend void deleteElement(T* p);

};




template<typename T, typename...Args>
T*  newElement(Args... arg)
{
    T* p;
    if((p = reinterpret_cast<T*>(HashBucket::usememory(sizeof(T)))) != nullptr)
        new(p) T(std::forward<Args>(arg)...);   //在p的空间上直接构造对象T，传入参数
    
    return p;
}

template<typename T>
void deleteElement(T* p)
{
    if(p)
    {
        p->~T();
        HashBucket::freememory(reinterpret_cast<void*>(p), sizeof(T));
    }
        
}


}
