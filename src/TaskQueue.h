#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

using Element = std::pair<std::unique_ptr<char[]>,size_t>;

class TaskQueue {
public:
    TaskQueue(size_t threadsNum = 1);

    ~TaskQueue();

    TaskQueue(const TaskQueue &&other) = delete;
    TaskQueue &operator=(const TaskQueue &&other) = delete;
    TaskQueue(TaskQueue &&other) = delete;
    TaskQueue &operator=(TaskQueue &&other) = delete;

    void push(Element&& element);
    void wait();
    void checkException();

private:
    void pull();

private:
    std::vector<std::thread> m_workers;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic_bool m_stop;
    std::queue<Element> m_buffers;
    std::exception_ptr m_exception;
};
