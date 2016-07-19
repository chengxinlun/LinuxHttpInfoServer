#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <mutex>
#include <vector>
#include <thread>
#include <deque>
#include <condition_variable>


class ThreadPool;


class Worker
{
    public:
        Worker(ThreadPool &p): pool(p){};
        void operator()();
    private:
        ThreadPool &pool;
};

class ThreadPool
{
    public:
        ThreadPool(int num);
        ~ThreadPool();
        template<class T>
            void enqueue(T t);
    private:
        friend class Worker;
        std::vector<std::thread> workers; // Track the threads
        std::deque<std::function<void()> > tasks; // Task queue
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop; // If the pool is stopped
};
#endif
