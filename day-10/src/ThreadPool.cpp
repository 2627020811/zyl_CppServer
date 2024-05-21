#include "ThreadPool.h"

ThreadPool::ThreadPool(int size) : stop(false)
{
    for (int i = 0; i < size; i++)
    {
        threads.emplace_back(std::thread([this]()
                                         {
            while(true)
            {
                std::function<void()> task;
                // 这段代码中，大括号 {} 实际上定义了一个作用域，称为局部作用域或者局部范围。在 C++ 中，使用大括号可以创建一个新的作用域，其中定义的变量在离开这个作用域后会被销毁，这有助于控制变量的生命周期，避免变量的影响持续到不应该的地方。

                // 在这个例子中，大括号创建了一个局部作用域，其中定义了 std::lock_guard<std::mutex> 对象 lock。这个 lock 对象在创建时会自动锁定互斥量 mutex，在离开作用域时会自动释放锁。这样做的好处是确保互斥量 mutex 在设置 dataReady 为 true 期间处于锁定状态，从而保证线程安全性。
                {
                    std::unique_lock<std::mutex> lock(tasks_mtx);
                    // cv.wait(lock,[this](){ return stop || !tasks.empty(); });：这是一个条件变量的等待操作。线程会在这里阻塞，直到条件 stop || !tasks.empty() 成立。条件变量 cv 与互斥锁 lock 结合使用，用于线程间的同步。
                    cv.wait(lock,[this]() 
                    {
                        return stop||!tasks.empty(); //等待条件变量，条件为任务队列不为空或线程池停止
                    });
                    if(stop&&tasks.empty()) return; //任务队列为空并且线程池停止，退出线程
                    task=tasks.front(); //从任务队列头取出一个任务
                    tasks.pop();
                }
                task(); //执行任务
            } }));
    }
}

ThreadPool::~ThreadPool()
{
    { ////在这个{}作用域内对std::mutex加锁，出了作用域会自动解锁，不需要调用unlock()
        std::unique_lock<std::mutex> lock(tasks_mtx);
        stop = true;
    }

    cv.notify_all();
    for (std::thread &th : threads)
    {
        if (th.joinable())
        {
            th.join();
        }
    }
}

void ThreadPool::add(std::function<void()> func)
{
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        if (stop)
            throw std::runtime_error("ThreadPool already stop, can't add task any more");
        tasks.emplace(func);
    }
    cv.notify_one();
}