
#include "Reader.h"

Reader::Reader(const std::string &path, size_t partSize):
    m_filesize(0),m_offset(0),m_partSize(partSize)
{
    auto mode = read_only;
    if(!std::filesystem::exists(path)){
        throw std::invalid_argument("Input file is not exists");
    }

    m_file = std::make_unique<file_mapping>(path.c_str(),mode);

    if(!m_file){
        throw std::runtime_error("File was`t opened");
    }

    m_region = std::make_unique<mapped_region>(*m_file,mode);
    m_filesize = m_region->get_size();
}

char *Reader::get() const {
    char * addr = static_cast<char *>(m_region->get_address()) + m_offset;

    if(!addr){
        throw std::runtime_error("Pointer is not valid");
    }

    return addr;
}

std::vector<std::pair<char *, size_t>> Reader::getPointers() {
    const size_t fullParts = m_filesize / m_partSize;
    const size_t tail = m_filesize % m_partSize;

    std::vector<size_t> parts(fullParts,m_partSize);
    if(tail > 0){
        parts.push_back(tail);
    }

    std::vector<std::pair<char*,size_t>> result(parts.size());

    for (int i=0; i < parts.size(); i++){
        auto& pair = result.at(i);
        pair.first = get();
        pair.second = parts.at(i);
        m_offset += parts.at(i);
    }
    return result;
}
