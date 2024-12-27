#include <pthread.h>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <vector>
using namespace std;

// Class that represents a simple thread pool
class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();
    void enqueue(function<void()> task);

private:
    static void* worker(void* arg);
    void run();

    vector<pthread_t> threads_;
    queue<function<void()>> tasks_;
    mutex queue_mutex_;
    condition_variable cv_;
    bool stop_ = false;
};

ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        pthread_t thread;
        pthread_create(&thread, nullptr, ThreadPool::worker, this);
        threads_.emplace_back(thread);
    }
}

ThreadPool::~ThreadPool() {
    {
        lock_guard<mutex> lock(queue_mutex_);
        stop_ = true;
    }
    cv_.notify_all();
    for (pthread_t& thread : threads_) {
        pthread_join(thread, nullptr);
    }
}

void ThreadPool::enqueue(function<void()> task) {
    {
        lock_guard<mutex> lock(queue_mutex_);
        tasks_.emplace(move(task));
    }
    cv_.notify_one();
}

void* ThreadPool::worker(void* arg) {
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    pool->run();
    return nullptr;
}

void ThreadPool::run() {
    while (true) {
        function<void()> task;
        {
            unique_lock<mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return !tasks_.empty() || stop_; });
            if (stop_ && tasks_.empty()) {
                return;
            }
            task = move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}
