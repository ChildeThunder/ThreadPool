#include "ThreadPool.h"

#include <stdexcept>
#include <utility>
using namespace std;
// 1. 构造函数
ThreadPool::ThreadPool(size_t threadCount)
{
    // TODO:
    // 检查 threadCount 是否为 0
    // 创建 threadCount 个工作线程
    // 每个线程运行 workerLoop()
    // 考虑创建线程过程中抛异常的清理
    if (threadCount == 0)
    {
        throw runtime_error("线程数量为0");
    }
    workers_.reserve(threadCount);
    try
    {
        for (int i = 0; i < threadCount; i++)
        {
            workers_.emplace_back(
                &ThreadPool::workerLoop,
                this);
        }
    }
    catch (...)
    {
        unique_lock<mutex> lk(mtx_);
        stopping_ = true;
        lk.unlock();
        cv_.notify_all();
        for (int i = 0; i < workers_.size(); i++)
        {
            if (workers_[i].joinable())
                workers_[i].join();
        }
        throw;
    }
}

// 2. 析构函数
ThreadPool::~ThreadPool()
{

    shutdown();
}

// 3. 提交任务
void ThreadPool::post(std::function<void()> task)
{
    // TODO:
    // 获取互斥锁
    // 检查线程池是否已经停止
    // 将 task 添加到 tasks_
    // 通知一个工作线程
    unique_lock<mutex> lk(mtx_);
    if (this->stopping_ == true)
    {
        throw runtime_error("线程池已经停止");
    }
    this->tasks_.push(move(task));
    lk.unlock();
    this->cv_.notify_one();
}

// 4. 工作线程循环
void ThreadPool::workerLoop()
{
    // TODO:
    // 不断循环
    // 获取互斥锁
    // 如果没有任务，使用条件变量等待
    // 如果已经停止且任务队列为空，则退出
    // 从队列取出一个任务
    // 释放锁
    // 执行任务
    while (1)
    {
        unique_lock<mutex> lk(mtx_);
        cv_.wait(lk, [this]()
                 {
            //“线程是否应该结束等待，醒过来检查一下当前状态？”
            return stopping_||!tasks_.empty(); });

        if (stopping_ && tasks_.empty())
        {
            break;
        }
        auto task = move(tasks_.front());
        tasks_.pop();
        lk.unlock();
        try
        {
            task();
        }
        catch (const exception &e)
        {
            cerr << "Task 失败：" << e.what() << endl;
        }
        catch (...)
        {
            cerr << "task未知错误" << endl;
        }
    }
}

// 5. 关闭线程池
void ThreadPool::shutdown()
{
    // TODO:
    // 将 stopping_ 设置为 true
    // 唤醒所有工作线程
    // 等待线程结束
    // 确保多次调用也安全
    unique_lock<mutex> lk(mtx_);
    stopping_ = true;
    lk.unlock();
    cv_.notify_all();
    for (int i = 0; i < workers_.size(); i++)
    {
        if (workers_[i].joinable())
            workers_[i].join();
    }
}