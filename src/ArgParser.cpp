#include "ArgParser.h"
#include <iostream>
#include <cstring>
#include <string>


ArgParser::ArgParser(int argc, char** argv) :
        m_sizeBlock(0),
        m_hwConcurency(0),
        m_inFilePath(""),
        m_outFilePath(""),
        m_verbose(false)
{

    po::options_description desc("short description: ");

    desc.add_options()
            ("help,h","help \n")
            ("input,i",po::value<std::string>()->required(),"Input file")
            ("output,o", po::value<std::string>()->required(), "Output file")
            ("size-block,s", po::value<uint32_t>()->default_value(1024), "Size block (bytes)")
            ("verbose,v", po::bool_switch(&m_verbose)->default_value(false), "Verbose");

    po::variables_map vm;

    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (po::error& e)
    {
        std::cout<<desc<<std::endl;

        throw std::invalid_argument(e.what());
    }


    /** Вывод справочного сообщения */
    if (vm.count("help")){
        std::cout<<desc<<std::endl;
    }

    /** Получение пути расположения файла */
    if(vm.count("input")){
        this->m_inFilePath = fs::path(vm["input"].as<std::string>());
    }

    if (vm.count("output")) {
        this->m_outFilePath = fs::path(vm["output"].as<std::string>());
    }

    if (vm.count("size-block")) {
        this->m_sizeBlock = vm["size-block"].as<uint32_t>();
    }

    if (!fs::exists(m_inFilePath)) {
        throw std::invalid_argument("Input file is not exists");
    }
}
