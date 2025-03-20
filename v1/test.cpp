#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include "memory_pool.h"

using namespace memorypools;

class P1
{
public:
    int test_;
};

class P2
{
    int test_[5];
};

class P3
{
    int test_[10];
};

class P4
{
    int test_[20];
};


void test_memorypool(size_t ntimes, size_t nworks, size_t rounds)
{
    std::vector<std::thread> vthread(nworks);
    auto total_time = 0ns;

    for(size_t i = 0 ; i < nworks ; i ++)
    {
        vthread[i] = std::thread([&]()
        {
            for(size_t j = 0; j < rounds; j ++)
            {
                auto begint = chrono::high_resolution_clock::now();

                for(size_t k = 0; k < ntimes; k ++)
                {
                    P1* p1 = newElement<P1>();
                    deleteElement<P1>(p1);
                    P2* p2 = newElement<P2>();
                    deleteElement<P2>(p2);
                    P3* p3 = newElement<P3>();
                    deleteElement<P3>(p3);
                    P4* p4 = newElement<P4>();
                    deleteElement<P4>(p4);                    
                }

                auto endt = chrono::high_resolution_clock::now();
                total_time += chrono::duration_cast<chrono::nanoseconds> (endt - begint);
            }
        }
        );

    }
    for(auto& th : vthread)
        th.join();
    
    printf("%llu 个线程  执行 %llu 轮  每轮申请&释放 %llu \n总计花费时间 < %llu > \n", nworks, rounds, ntimes, total_time.count());
}



void test_newanddelete(size_t ntimes, size_t nworks, size_t rounds)
{

    std::vector<std::thread> vthread(nworks);
    
    auto total_time = 0ns;

    for(size_t i = 0 ; i < nworks ; i ++)
    {
        vthread[i] = std::thread([&]()
        {
            for(size_t j = 0; j < rounds; j ++)
            {
                auto begint = chrono::high_resolution_clock::now();

                for(size_t k = 0; k < ntimes; k ++)
                {
                    P1* p1 = new P1;
                    delete p1;
                    P2* p2 = new P2;
                    delete p2;
                    P3* p3 = new P3;
                    delete p3;
                    P4* p4 = new P4;
                    delete p4;
                }

                auto endt = chrono::high_resolution_clock::now();
                total_time += chrono::duration_cast<chrono::nanoseconds>(endt - begint);
            }
        }
        );

    }
    for(auto& th : vthread)
        th.join();
    
    printf("%llu 个线程  执行 %llu 轮  每轮申请&释放 %llu 总计花费时间< %llu >\n", nworks, rounds, ntimes, total_time.count());
}


int main()
{
    HashBucket::init_memorypool();
    test_memorypool(100, 1, 1000);
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    test_newanddelete(100, 1, 1000);

    return 0;
}

//debug时间10h 写代码 4h 理解框架 3h
