#include "Writer.h"
#include <iostream>
#include <fstream>

Writer::Writer(const std::string &path, size_t filesize,size_t blockSize):
    m_offset(0),m_blockSize(blockSize)
{
    auto mode = read_write;

    {  //Create a file
        file_mapping::remove(path.c_str());
        std::filebuf fbuf;
        fbuf.open(path.c_str(), std::ios_base::in | std::ios_base::out
                            | std::ios_base::trunc | std::ios_base::binary);
        //Set the size
        fbuf.pubseekoff(filesize-1, std::ios_base::beg);
        fbuf.sputc(0);
    }

    //Create a file mapping
    m_file = std::make_unique<file_mapping>(path.c_str(),mode);

    if (!m_file) {
        throw std::runtime_error("File was`t opened");
    }
    //Map the whole file with read-write permissions in this process
    m_region = std::make_unique<mapped_region>(*m_file, mode);
}

char *Writer::get(uint32_t offset) {

    size_t inOffset = offset > 0? offset : m_offset;
    char * addr = static_cast<char *>(m_region->get_address()) + inOffset;

    if(!addr){
        throw std::runtime_error("Pointer is not valid");
    }

    m_offset += inOffset;

    return addr;
}
