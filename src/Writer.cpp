//
// Created by solovjev-y on 03.02.2020.
//

#include <iostream>
#include "Writer.h"

Writer::Writer(const std::string &outputFile):m_stop(false) {

    /**Checking input path */
    if (outputFile.empty()) {
        throw std::runtime_error("Output file`s path is empty ");
    }

    /** Opens the output file*/
    m_output = std::make_unique<std::ofstream>(outputFile, std::ios::binary | std::ios_base::out);

    if (!m_output) {
        throw std::runtime_error("Output file is not available");
    }

    /** To launch thread for write in file */
    m_thread = std::thread(&Writer::exec,this);

}

void Writer::exec() {

    try {
        while (!m_stop || !m_data.empty()) {

            std::unique_lock _lock(m_mutex);

            m_cv.wait(_lock,[this](){
                return m_stop || !m_data.empty();
            });

            if(!m_data.empty()) {
                m_output->write(reinterpret_cast<char *>(m_data.data()), m_data.size() * sizeof(uint32_t));
                m_data.clear();
            }
        }
    }
    catch (...){
       m_exception = std::current_exception();
    }
}

void Writer::push(const uint32_t &&value) {

    {
        std::lock_guard _lock(m_mutex);
        m_data.push_back(value);
    }

    m_cv.notify_one();
}

void Writer::checkException() {
}

Writer::~Writer() {
    if(m_thread.joinable()){
        m_stop = true;
        m_cv.notify_all();
        m_thread.join();
    }
}
