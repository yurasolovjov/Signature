#include <iostream>
#include <boost/crc.hpp>
#include "TaskQueue.h"


TaskQueue::TaskQueue(Writer& writer, size_t threadsNum):
m_writer(writer),m_stop(false)
{
    for (int i=0; i < threadsNum; i++){
        m_workers.emplace_back(&TaskQueue::pull, this);
    }
}

void TaskQueue::pull() {

    while(!m_stop || !m_buffers.empty()){

        std::unique_lock _lock(m_mutex);

        m_cv.wait(_lock,[&]()->bool
        {
            return (!m_buffers.empty() || m_stop );
        });

        try {
            if(!m_buffers.empty()){
                boost::crc_32_type crc;

                {
                    /** Memory will delete*/
                    auto element = std::move(m_buffers.front());
                    m_buffers.pop();
                    /** Unlock mutex*/
                    _lock.unlock();

                    size_t size = element.second;
                    crc.process_bytes(element.first.get(), size);
                }

                m_writer.push(std::move(crc.checksum()));

            } else if (m_stop || m_buffers.empty()) {
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

void TaskQueue::push(Element&& element) {
    std::lock_guard _lock(m_mutex);
    m_buffers.emplace(std::move(element));
    m_cv.notify_all();
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

