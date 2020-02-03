#pragma once

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <vector>
#include <filesystem>

namespace po = boost::program_options;
namespace fs = std::filesystem;


class ArgParser{

public:

    explicit ArgParser(int argc, char** argv);

    ~ArgParser(){};

    ArgParser(ArgParser&&) = delete;
    ArgParser& operator=(ArgParser&&) = delete;

public:

    const fs::path& getInputFile() const { return m_inFilePath; }
    const fs::path& getOutputFile() const { return m_outFilePath; }
    const uint32_t getSizeBlock() const{ return m_sizeBlock; };
    bool isVerbose()const { return m_verbose; };

private:

    bool m_verbose;
    uint16_t  m_hwConcurency;
    uint32_t  m_sizeBlock;
    fs::path m_inFilePath;
    fs::path m_outFilePath;
};

