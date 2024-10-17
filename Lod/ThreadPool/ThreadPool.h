#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <functional>

#define THREAD_POOL_THREAD_COUNT 32

class ThreadPool {
private:
    bool terminate = false;

    std::mutex queue_mutex;
    std::condition_variable mutex_condition;

    std::mutex counter_mutex;
    unsigned int busy_threads = 0;

    std::vector<std::thread> threads;
    std::queue<std::pair<std::function<void(void*)>, void*>> jobs;

    void ThreadLoop();
public:
    ThreadPool() {}
    void Start();
    void QueueJob(const std::function<void(void*)> job);

    template <class T>
    void QueueJob(const std::function<void(void*)>& job, const T& args) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            jobs.push(std::pair<std::function<void(void*)>, void*>(job, new T(args)));
        }
        mutex_condition.notify_one();
    }

    void Stop();
    bool IsBusy();
    bool IsIdle();
    void Join();
};