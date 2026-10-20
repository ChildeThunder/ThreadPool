#include "ThreadPool.h"
#include "MockModel.h"

#include <iostream>
#include <thread>
#include <future>
#include <atomic>
#include <vector>
#include <chrono>
#include <stdexcept>
using namespace std;
void test01()
{
        std::mutex coutMtx;

    // 创建3个工作线程
    ThreadPool pool(3);

    // 提交10个推理任务
    for (int i = 0; i < 10; ++i) {
        pool.post([i, &coutMtx]() {

            auto result = simulateInfer(
                i,
                {1.0f, 2.0f, 3.0f}
            );

            std::lock_guard<std::mutex> lock(coutMtx);

            std::cout
                << "Task " << result.task_id
                << " score = " << result.score
                << " thread = " << std::this_thread::get_id()
                << std::endl;
        });
    }

    // 等待所有任务完成后关闭
    pool.shutdown();
}
void test02()
{
ThreadPool pool(3);

    auto f1 = pool.submit([]() {
        return 10 + 20;
    });

    auto f2 = pool.submit([]() {
        return 100 + 200;
    });

    cout << f1.get() << endl;
    cout << f2.get() << endl;

    pool.shutdown();
}
void test03()
{
    ThreadPool pool(3);

    // 正常任务
    auto f1 = pool.submit([]() {
        return 100 + 200;
    });

    // 故意抛异常的任务
    auto f2 = pool.submit([]() -> int {
        throw runtime_error("AI推理失败");
    });

    cout << "主线程继续执行" << endl;

    cout << "正常结果：" << f1.get() << endl;

    try {
        cout << "异常任务结果：" << f2.get() << endl;
    }
    catch (const exception& e) {
        cout << "捕获异常：" << e.what() << endl;
    }

    pool.shutdown();
}
int main()
{
    // ========= 测试1：优雅关闭 =========
    cout << "===== Test 1 =====" << endl;

    atomic<int> finished{0};

    {
        ThreadPool pool(3);

        for (int i = 0; i < 20; ++i)
        {
            pool.post([&finished]() {
                this_thread::sleep_for(
                    chrono::milliseconds(20)
                );

                ++finished;
            });
        }

        // 不主动调用 shutdown()
        // 直接离开作用域
    }

    cout << "Finished: " << finished << endl;


    // ========= 测试2：future 与异常 =========
    cout << "===== Test 2 =====" << endl;

    {
        ThreadPool pool(3);

        auto f1 = pool.submit([]() {
            return 100;
        });

        auto f2 = pool.submit([]() -> int {
            throw runtime_error("Inference failed");
        });

        auto f3 = pool.submit([]() {
            return 300;
        });

        cout << "Result 1: " << f1.get() << endl;

        try
        {
            f2.get();
        }
        catch (const exception& e)
        {
            cout << "Caught: " << e.what() << endl;
        }

        cout << "Result 3: " << f3.get() << endl;

        pool.shutdown();
    }


    // ========= 测试3：并发提交 =========
    cout << "===== Test 3 =====" << endl;

    {
        ThreadPool pool(3);

        atomic<int> completed{0};
        vector<thread> producers;

        // 模拟4个用户线程同时提交任务
        for (int i = 0; i < 4; ++i)
        {
            producers.emplace_back([&pool, &completed]() {

                for (int j = 0; j < 50; ++j)
                {
                    pool.post([&completed]() {
                        ++completed;
                    });
                }

            });
        }

        // 等待4个提交者提交完毕
        for (auto& t : producers)
        {
            t.join();
        }

        pool.shutdown();

        cout << "Completed: " << completed << endl;
    }


    // ========= 测试4：关闭后提交 =========
    cout << "===== Test 4 =====" << endl;

    {
        ThreadPool pool(3);

        pool.shutdown();

        // 重复关闭
        pool.shutdown();

        try
        {
            pool.post([]() {
                cout << "Should not run" << endl;
            });

            cout << "ERROR: accepted new task" << endl;
        }
        catch (const exception& e)
        {
            cout << "Rejected: " << e.what() << endl;
        }
    }

    return 0;
}