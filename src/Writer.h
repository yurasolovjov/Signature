#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

class Writer {

public:
    Writer(const std::string& outputFile);
    ~Writer();

    Writer(const Writer& ) = delete;
    Writer(Writer&&) = delete;
    Writer& operator=(Writer&) = delete;
    Writer& operator=(Writer&&) = delete;

    void exec();
    void push(const uint32_t&& value);
    void checkException();

private:
    std::thread m_thread;
    std::unique_ptr<std::ostream> m_output;
    std::exception_ptr m_exception;
    std::mutex m_mutex;
    std::vector<uint32_t> m_data;
    std::condition_variable m_cv;
    std::atomic_bool m_stop;
};


