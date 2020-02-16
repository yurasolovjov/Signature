#pragma once

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <string>
#include <memory>
#include <filesystem>

using namespace boost::interprocess;

class Writer {
public:

    Writer() = delete;
    Writer(Writer&&) = delete;
    Writer& operator=(Writer&&) = delete;

    /** @brief Class to split input file on pointers with offset equal part size
     *  @param path input file
     *  @param partSize size of memory`s block
     * */
    explicit Writer(const std::string& path,size_t filesize, size_t blockSize = 1024);

    /** @brief Method return pointer to the memory`s block*/
    char * get(uint32_t offset = 0) ;

private:
    std::unique_ptr<file_mapping> m_file;
    std::unique_ptr<mapped_region> m_region;
    size_t m_offset;
    size_t m_blockSize;
};

