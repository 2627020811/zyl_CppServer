#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool
{
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mtx;
    std::condition_variable cv;
    bool stop;

public:
    ThreadPool(int size = 10);
    ~ThreadPool();
    // 以下两句代码定义了一个函数模板 add，它的目的是将一个可调用对象（比如函数、函数对象、lambda 表达式等）与其参数一起包装成一个异步任务，并返回一个 std::future 对象，以便在将来获取任务的执行结果。
    template <class F, class... Args>
    // std::future 是 C++ 标准库中的一个模板类，它提供了一种访问异步操作结果的方法。
    // std::result_of<F(Args...)>::type 用于获取调用 f(args...) 的结果类型
    // 在模板代码中，std::result_of<F(Args...)>::type 是一个依赖类型（dependent type），因为它依赖于模板参数 F 和 Args...。C++ 编译器无法确定 std::result_of<F(Args...)>::type 是一个类型名还是一个数据成员，因此我们必须显式地使用 typename 来告诉编译器这是一个类型名。
    auto add(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;
    // typename std::result_of<F(Args...)>::type 将 std::result_of<F(Args...)> 结果的类型提取出来，并使用它作为 std::future 的模板参数，表示 std::future 将会保存 f 调用后的结果
};

// 不能放在cpp文件，原因是C++编译器不支持模版的分离编译
template <class F, class... Args>
auto ThreadPool::add(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        // don't allow enqueueing after stopping the pool
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]()
                      { (*task); });
    }
    cv.notify_one();
    return res;
}