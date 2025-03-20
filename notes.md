# 说明

本文档用于记录project实现过程中的 实现目标  问题及解决  时间  思考


# project 过程


理解框架 -->  代码实现 --> 代码调试   |--> 代码优化

                            ||
                        学习VScode配置文件
                        学习Cmake --> 拉长周期 避免影响到主要计划！！
                        v1-未优化版本完成测试 性能不如直接分配！

记录时间 3.20


# 实现目标

memorypools 提供两个 接口 ：
- newElement 
- deleteElement
  

---
# 时间

理解框架3h
实现代码3h
debug 3h

---
# C++语法

1. namespace {}


2. 头文件定义 模板函数的定义


3. 静态函数 vs 类方法：

静态函数（@staticmethod）不需要类或实例的引用。

类方法（@classmethod）第一个参数是类本身（cls），可以访问类属性。


4. 在声明函数时可以只传入参数类型，定义必须传参数名


5. 类的定义一定需要在右括号位置加上;
   即 `struct try {};`

6. assert(size > 0) 是 C/C++ 中用于调试的断言（Assertion），
它的核心作用是：在程序运行时检查条件 size > 0 是否成立。如果条件不成立（即 size <= 0），程序会立即终止，并输出错误信息。



7. 成员初始化列表语法

BlockSize_(BlockSize)
表示将构造函数的参数 BlockSize 赋值给类的成员变量 BlockSize_。
这种写法避免了成员变量与参数名冲突（例如成员变量加 _ 后缀是常见约定）


8. operator delete(reinterpret_cast<void*>(cur));  
operator delete 直接释放 operator new 分配的空间，不调用对象的析构函数

reinterpret_cast< >() 将括号内的变量的类型转换为<>中的类型


9. std::lock_guard<std::mutex> lock(mutexforfreelist); 

核心功能：自动管理互斥锁
​自动加锁：
在构造 lock_guard 对象时，会立即对 mutexforfreelist 加锁（调用 mutex.lock()），确保当前线程独占访问共享资源。
​自动解锁：
当 lock_guard 对象离开作用域（如函数结束、代码块退出或异常抛出时），自动释放锁（调用 mutex.unlock()），​避免忘记手动解锁导致的死锁。



10. 在 C++ 中，void* 是一种 ​通用指针类型，表示“指向未知类型数据的指针”。它的核心特性和用法如下：

​1. void* 的核心特性
​无类型指针：不关联任何具体数据类型，仅表示一个内存地址。
​强制类型转换：必须显式转换为具体类型指针（如 int*、MyClass*）后才能访问数据。
​不参与类型检查：编译器无法验证其指向的数据类型，需开发者自行确保安全。


11. 友元
在 C++ 中，友元（Friend） 是一种打破类封装性的机制，允许某些外部函数或类访问当前类的私有（private）或保护（protected）成员。它的核心作用是在特定场景下提供灵活的数据共享，但同时需谨慎使用以保持代码的健壮性。


12. vthread[k] = std::thread([&]() { ... });
作用：为 vthread 的第 k 个元素分配一个新线程，该线程执行 lambda 表达式中的任务。

语法分解：

vthread[k]：访问 vthread 的第 k 个元素（一个 std::thread 对象）。

std::thread([&]() { ... })：构造一个 std::thread 对象，参数是一个 lambda 表达式。

[&]：lambda 捕获列表，表示以 引用方式捕获所有外部变量。

()：lambda 参数列表（此处无参数）。

{ ... }：lambda 函数体，定义线程要执行的任务。

赋值操作：通过移动赋值运算符（std::thread 不可复制但可移动）将新线程的所有权转移到 vthread[k]。


13. std::vector<int> a(10);   // 10 个 0
    std::vector<int> b{10};   // 1 个 10
    std::vector<int> b[10];   // 10 个vector<int> 数组



14. 在 C++ 多线程编程中，j.join() 表示 ​阻塞当前线程（通常是主线程），直到线程 j 执行完毕。以下是详细解释：

1.核心作用
​同步线程：调用 j.join() 的线程（如主线程）会暂停执行，等待线程 j 完成其任务。
​资源回收：确保线程 j 结束后，其资源（如栈内存、线程句柄）被正确释放。
2.对比 j.detach()
​方法	​行为	​适用场景
j.join()	阻塞当前线程，等待 j 结束。	需要线程执行结果的场景。
j.detach()	将 j 分离为后台线程，生命周期与主线程无关。	不关心线程何时结束的后台任务。




---
# 实验问题

1. 如何定义namespace？

c++中有内置的namespace 关键字 直接类似于结构体一样去 定义即可

```
namespace mynamespace 
{
    int hh = 0;
    void print()
    {
        cout << hh << endl;
    }
}
```

2. size_t 是什么意思？

在C/C++中 ，size_t 是先预定义的无符号整数类型  
作用：为内存相关操作提供与平台无关的便准话类型

得先include标准头文件 如 ：
- <stddef.h>
- <cstddef>
- <studio.h>
等


3. 为什么newElement和deleteElement函数要放在头文件中？

因为模板函数必须放在头文件中，这是模板的**编译**机制决定的。


4. 在init函数中
```
    tmp = curSlot_;
    curSlot_ = curSlot_->next;
    return tmp;
```
为神马不对？

因为实际上cur的移动并非依靠链表，而是依靠分配的地址时连续的 



5. vscode启用调试模式total_time输出正确的结果，而如果直接运行就会得出一个错误的值或者直接报错

目前，好像是vscode的C/C++的调试配置文件的问题，不能编译多个cpp文件，这个需要去官网上学配置文件  或者使用Cmake makefile 来进行编译

使用cmake进行多文件的编译， 注意要选择对应的编译器  通过 打开命令搜索（ctrl + shift + P） cmake : configure 选择对应的编译器

然后使用 mingw32-make 编译 


---
# 思考


1. 为什么需要 Slot_freeSlot ? 为什么不直接归入当前的可使用链表？

项目是memorypools 当前指向的block就是一个内存池，但是可能之前的内存池中一已经有对象释放归还空间，所以需要单独设立一个 Slot_freeSlot 自由链表记录整个pools的已分配的空间！



2. hash函数的作用是什么?

每个桶 也就是 内存池的整体大小是一致的 ，但是每个内存池的划分的Slotsize不同 为 8B, 16B等等！！
所以hash函数是为了快速定位实际的对应大小的内存池地位置



3. 为什么allocate不需要传入参数？
因为每个pool的Slot是固定的，直接分配就行



4. 为什么是cur = last 的时候就分配内存？

因为要提前进行申请，当抵达最后一个Slot的时候就需要申请，且一定是在分配前就申请，因为cur会在分配时指向下一块地址

注意最后一块Slot在当前的实现方式下 不会被使用！！！