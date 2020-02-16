#include <iostream>
#include <boost/crc.hpp>
#include "TaskQueue.h"


TaskQueue::TaskQueue(const size_t threadsNum):
m_stop(false)
{
    for (int i=0; i < threadsNum; i++){
        m_workers.emplace_back(&TaskQueue::pull, this);
    }
}

void TaskQueue::pull() {

    while(!m_stop || !m_tasks.empty()){

        std::unique_lock _lock(m_mutex);

        m_cv.wait(_lock,[&]()->bool
        {
            return (!m_tasks.empty() || m_stop );
        });

        try {
            if(!m_tasks.empty()){
                boost::crc_32_type crc;

                {
                    /** Memory will delete*/
                    auto task = std::move(m_tasks.front());
                    m_tasks.pop();

                    /** Unlock mutex*/
                    _lock.unlock();

                    task();
                }

            } else if (m_stop || m_tasks.empty()) {
                break;
            }
        }
        catch (...){
            m_exception = std::current_exception();
        }
    }
}

TaskQueue::~TaskQueue() {
}

void TaskQueue::wait(){
    m_stop = true;
    m_cv.notify_all();

    for (auto &worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void TaskQueue::push(Task&& element) {
    std::lock_guard _lock(m_mutex);
    m_tasks.emplace(std::move(element));
    m_cv.notify_one();
}

void TaskQueue::checkException() {
    try {
        if (m_exception) {
            std::rethrow_exception(m_exception);
        }
    }
    catch (const std::exception& e){
        throw std::runtime_error("*** Exception *** Internal error");
    }
}

