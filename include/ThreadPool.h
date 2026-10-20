#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <future>
#include<memory>
#include<string>
class ThreadPool
{
private:
    // 工作线程
    std::vector<std::thread> workers_;

    // 任务队列
    std::queue<std::function<void()>> tasks_;

    // 互斥锁
    std::mutex mtx_;

    // 条件变量
    std::condition_variable cv_;

    // 停止标志
    bool stopping_ = false;

    // 工作线程执行的函数
    void workerLoop();

public:
    // 构造函数
    explicit ThreadPool(size_t threadCount);

    // 析构函数
    ~ThreadPool();

    // 禁止拷贝
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    // 提交任务
    void post(std::function<void()> task);

    // 关闭线程池
    void shutdown();
    // Day 6 新增：模板函数放在 .h 中
    template <typename F>
    auto submit(F &&f)
        -> std::future<std::invoke_result_t<F>>
    {
        using R = std::invoke_result_t<F>;

        // 1. 包装任务
        auto taskPtr =
            std::make_shared<std::packaged_task<R()>>(
                std::forward<F>(f));

        // 2. 获取 future
        std::future<R> res = taskPtr->get_future();

        // 3. 提交到线程池
        post([taskPtr]()
             { (*taskPtr)(); });

        // 4. 返回 future
        return res;
    }
    std::future<int> submitInt(std::function<int()> f)
    {
        auto taskp =
            std::make_shared<std::packaged_task<int()>>(
                std::move(f));

        std::future<int> res = taskp->get_future();

        post([taskp]()
             { (*taskp)(); });

        return res;
    }

    std::future<std::string> submitString(
        std::function<std::string()> f)
    {
        auto taskp =
            std::make_shared<std::packaged_task<std::string()>>(
                std::move(f));

        std::future<std::string> res = taskp->get_future();

        post([taskp]()
             { (*taskp)(); });

        return res;
    }
};