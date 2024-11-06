#pragma once

#include <assert.h>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
private:
    using Task_t = std::function<void()>;
    using Thread_t = std::thread;
    using size_t = std::size_t;

private:
    std::vector<Thread_t> _works;
    std::queue<Task_t> _tasks;
    std::atomic<bool> _stop;
    std::condition_variable _cond;
    std::mutex _mutex;

public:
    ThreadPool() = delete;
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    ThreadPool(size_t size)
    {
        assert(size != 0);
        _stop = false;

        for (auto i = 0; i < size; ++i) {
            _works.emplace_back([this] {
                while (!_stop) {
                    Task_t task;
                    {
                        std::unique_lock<std::mutex> _lock(_mutex);
                        _cond.wait(_lock, [&] { return !_tasks.empty() || _stop; });

                        if (_stop) {
                            return;
                        }

                        task = _tasks.front();
                        _tasks.pop();
                    }

                    if (task) {
                        task();
                    }
                }
            });
        }
    }

    // template <typename F, typename... Args>
    // auto submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))>
    // {
    //     using result_t = decltype(f(args...));
    //     auto task = std::make_shared<std::packaged_task<result_t()>>(
    //         std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    //     {
    //         std::unique_lock<std::mutex> _lock(_mutex);
    //         _tasks.push([task] { (*task)(); });
    //         _cond.notify_one();
    //     }

    //     return task->get_future();
    // }

    template <typename F, typename... Args>
    auto submit(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using result_t = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<result_t()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        {
            std::unique_lock<std::mutex> _lock(_mutex);
            _tasks.push([task] { (*task)(); });
            _cond.notify_one();
        }

        return task->get_future();
    }

    ~ThreadPool()
    {
        _stop = true;
        _cond.notify_all();

        for (auto &work : _works) {
            if (work.joinable()) {
                work.join();
            }
        }
    }
};
