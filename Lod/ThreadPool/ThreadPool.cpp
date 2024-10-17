#include "Threadpool.h"

void ThreadPool::ThreadLoop() {
    for (;;) {
        std::pair<std::function<void(void*)>, void*> job_with_args;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this] {
                return !jobs.empty() || terminate;
            });
            if (terminate) {
                return;
            }
            job_with_args = jobs.front();
            jobs.pop();
        }
        {
            std::unique_lock<std::mutex> lock(counter_mutex);
            busy_threads += 1;
        }

        job_with_args.first(job_with_args.second);
        delete job_with_args.second;

        {
            std::unique_lock<std::mutex> lock(counter_mutex);
            busy_threads -= 1;
        }
    }
}

void ThreadPool::Start() {
	threads.reserve(THREAD_POOL_THREAD_COUNT);
	for (int i = 0; i < THREAD_POOL_THREAD_COUNT; i++) {
		threads.push_back(std::thread(&ThreadPool::ThreadLoop, this));
	}
}

void ThreadPool::QueueJob(const std::function<void(void*)> job) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(std::pair<std::function<void(void*)>, void*>(job, nullptr));
    }
    mutex_condition.notify_one();
}

void ThreadPool::Stop() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread& active_thread : threads) {
        active_thread.join();
    }
    threads.clear();
}

bool ThreadPool::IsBusy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        poolbusy = !jobs.empty();
    }
    return poolbusy;
}

bool ThreadPool::IsIdle() {
    bool is_idle;
    {
        std::unique_lock<std::mutex> lock_queue(queue_mutex);
        std::unique_lock<std::mutex> lock_counter(counter_mutex);

        is_idle = jobs.empty() && (busy_threads == 0);
        printf("Job queue is: %s, busy thread count is: %d\n", jobs.empty() ? "Empty" : "Not empty", busy_threads);
    }
    return is_idle;
}

void ThreadPool::Join() {
    while (!this->IsIdle()) {
        // Busy looping
    }
    this->Stop();
}