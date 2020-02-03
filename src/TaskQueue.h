#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <Writer.h>

using Task = std::function<void(void)>;

class TaskQueue {
public:
    TaskQueue(const size_t threadsNum = 1);

    ~TaskQueue();

    TaskQueue(const TaskQueue &&other) = delete;
    TaskQueue &operator=(const TaskQueue &&other) = delete;
    TaskQueue(TaskQueue &&other) = delete;
    TaskQueue &operator=(TaskQueue &&other) = delete;

    void push(Task&& element);
    void wait();
    void checkException();

private:
    void pull();

private:
    std::vector<std::thread> m_workers;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic_bool m_stop;
    std::queue<Task> m_tasks;
    std::exception_ptr m_exception;
};
