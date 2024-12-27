#ifndef _THREADPOOL_HPP_
#define _THREADPOOL_HPP_

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

#endif