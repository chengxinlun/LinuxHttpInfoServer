#include "thread_pool.h"
#include <mutex>
#include <thread>

void Worker::operator()()
{
    std::function<void()> task;
    while (true)
    {
        // Acquire lock
        std::unique_lock<std::mutex> lock(pool.queue_mutex);
        // Wait for tasks
        while(!pool.stop && pool.tasks.empty())
        {
            pool.condition.wait(lock);
        }
        // Terminate thread when pool is stopped
        if (pool.stop)
            return;
        // Get task from queue
        task = pool.tasks.front();
        pool.tasks.pop_front();
        // Release lock
        lock.release();
    }
}


ThreadPool::ThreadPool(int num)
{
    for (int i = 0; i < num; i++)
        workers.push_back(std::thread(Worker(*this)));
}


ThreadPool::~ThreadPool()
{
    stop = true;
    condition.notify_all();
    for (size_t i = 0; i < workers.size(); i++)
        workers[i].join();
}


template<class T>
void ThreadPool::enqueue(T t)
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.push_back(std::function<void()>(t));
    lock.release();
    condition.notify_one();
}
