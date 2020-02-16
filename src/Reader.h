#pragma once

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <string>
#include <memory>
#include <filesystem>

using namespace boost::interprocess;

class Reader {
public:

    Reader() = delete;

    /** @brief Class to split input file on pointers with offset equal part size
     *  @param path input file
     *  @param partSize size of memory`s block
     * */
    explicit Reader(const std::string& path, size_t partSize = 1024);

    /** @brief Method to split an inpute file on pointers
     *  @return pair with pointer on memory`s block and block`s size
     * */
    std::vector<std::pair<char*,size_t>> getPointers();
private:
    char * get() const;

private:
    std::unique_ptr<file_mapping> m_file;
    std::unique_ptr<mapped_region> m_region;
    size_t m_offset;
    size_t m_filesize;
    size_t m_partSize;
    size_t m_numberOfParts;
};

