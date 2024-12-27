// Reference: https://www.geeksforgeeks.org/thread-pool-in-cpp/

// #include "ThreadPool.h"

// #include <condition_variable>
// #include <functional>
// #include <iostream>
// #include <mutex>
// #include <queue>
// #include <thread>
// using namespace std;

// Class that represents a simple thread pool
// class ThreadPool {
// public:
//     // // Constructor to creates a thread pool with given
//     // number of threads
//     ThreadPool(size_t num_threads)
//     {

//         // Creating worker threads
//         for (size_t i = 0; i < num_threads; ++i) {
//             threads_.emplace_back([this] {
//                 while (true) {
//                     function<void()> task;
//                     // The reason for putting the below code
//                     // here is to unlock the queue before
//                     // executing the task so that other
//                     // threads can perform enqueue tasks
//                     {
//                         // Locking the queue so that data
//                         // can be shared safely
//                         unique_lock<mutex> lock(queue_mutex_);

//                         // Waiting until there is a task to
//                         // execute or the pool is stopped
//                         cv_.wait(lock, [this] {
//                             return !tasks_.empty() || stop_;
//                         });

//                         // exit the thread in case the pool
//                         // is stopped and there are no tasks
//                         if (stop_ && tasks_.empty()) {
//                             return;
//                         }

//                         // Get the next task from the queue
//                         task = move(tasks_.front());
//                         tasks_.pop();
//                     }

//                     task();
//                 }
//             });
//         }
//     }

//     // Destructor to stop the thread pool
//     ~ThreadPool()
//     {
//         {
//             // Lock the queue to update the stop flag safely
//             unique_lock<mutex> lock(queue_mutex_);
//             stop_ = true;
//         }

//         // Notify all threads
//         cv_.notify_all();

//         // Joining all worker threads to ensure they have
//         // completed their tasks
//         for (auto& thread : threads_) {
//             thread.join();
//         }
//     }

//     // Enqueue task for execution by the thread pool
//     void enqueue(function<void()> task)
//     {
//         {
//             unique_lock<std::mutex> lock(queue_mutex_);
//             tasks_.emplace(move(task));
//         }
//         cv_.notify_one();
//     }

// private:
//     // Vector to store worker threads
//     vector<thread> threads_;

//     // Queue of tasks
//     queue<function<void()> > tasks_;

//     // Mutex to synchronize access to shared data
//     mutex queue_mutex_;

//     // Condition variable to signal changes in the state of
//     // the tasks queue
//     condition_variable cv_;

//     // Flag to indicate whether the thread pool should stop
//     // or not
//     bool stop_ = false;
// };

// std::mutex printMutex;

// void handle_connection(){
    
//     cout << this_thread::get_id() << ": do work\n" << endl;
// }

// int main()
// {
//     // Create a thread pool with 4 threads
//     ThreadPool pool(5);

//     // Enqueue tasks for execution
//     for (int i = 0; i < 10; ++i) {
//         pool.enqueue([i] {
            
//             std::lock_guard<std::mutex> guard(printMutex); 
//             cout << "Task " << i << " is running on thread "
//                  << this_thread::get_id() << endl;
//             // Simulate some work
//             handle_connection();
//             // this_thread::sleep_for(
//             //     chrono::milliseconds(100));
//         });
//     }

//     return 0;
// }



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

// std::mutex printMutex;

// void handle_connection() {
//     // lock_guard<mutex> guard(printMutex);
//     cout << pthread_self() << ": do work\n" << endl;
// }

// int main() {
//     ThreadPool pool(5);

//     for (int i = 0; i < 10; ++i) {
//         pool.enqueue([i] {
//             lock_guard<mutex> guard(printMutex);
//             cout << "Task " << i << " is running on thread " << pthread_self() << endl;
//             handle_connection();
//         });
//     }

//     return 0;
// }
