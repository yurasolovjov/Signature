#include <iostream>
#include "ArgParser.h"
#include <thread>
#include <filesystem>
#include "TaskQueue.h"
#include "Writer.h"
#include <deque>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        fs::path inputFilePath = "";
        std::string outputFilePath = "";
        uint32_t sizeBlock = 0;
        bool verbose = false;

        /** Parse arguments command lines */
        try {

            ArgParser args(argc, argv);

            inputFilePath = args.getInputFile();
            outputFilePath = args.getOutputFile();
            sizeBlock = args.getSizeBlock();
            verbose = args.isVerbose();

        }
        catch (std::invalid_argument &e) {

            std::cout << "*** ERROR *** Incorrect arguments. " << e.what() << std::endl;

            return EXIT_SUCCESS;
        }
        catch (std::exception& e){
            std::cout << "*** ERROR *** Incorrect  " << e.what() << std::endl;
            return EXIT_SUCCESS;
        }

        /** Get count of threads*/
        uint32_t hwConcurency = std::thread::hardware_concurrency();
        uint64_t fileSize = fs::file_size(fs::path(inputFilePath));
        std::cout << "Input file: " << inputFilePath << " (" << std::to_string(fileSize) << " bytes )" << std::endl;
        std::cout << "Output file: " << outputFilePath << std::endl;
        std::cout << "Size block: " << std::to_string(sizeBlock) << std::endl;
        std::cout << "Hardware concurency: " << std::to_string(hwConcurency) << std::endl;



        try {

            auto start = std::chrono::high_resolution_clock::now();
            /** Open the input file*/
            auto inputFile = std::make_unique<std::ifstream>(inputFilePath, std::ios::binary);

            if (!inputFile->is_open()) {
                throw std::runtime_error("Input file is not opened");
            }

            std::queue<std::pair<std::unique_ptr<char[]>,size_t>> m_buffers;

            /** Create writer*/
            Writer writer(outputFilePath);

            /** One thread for writer*/
            hwConcurency--;

            /** If system has a one core */
            hwConcurency = std::max<uint32_t >(hwConcurency,1);

            TaskQueue queue(writer, hwConcurency);

            while (inputFile->good()) {
                auto buffer = std::unique_ptr<char[]>(new char[sizeBlock]);
                /** Read from file to buffer */
                inputFile->read(buffer.get(), sizeBlock);
                size_t gcount = inputFile->gcount();
                /** Check internal exception */
                queue.checkException();
                queue.push(std::make_pair(std::move(buffer), gcount));
            }

            queue.wait();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;

            std::cout << "Time elapsed: " << elapsed.count() <<" milliseconds"<< std::endl;
        }
        catch (std::runtime_error& e){
            std::cerr<<e.what()<<std::endl;
            return EXIT_FAILURE;
        }
        catch (...){
            std::cout<<"*** ERROR *** Unknown exception "<< std::endl;
            return EXIT_FAILURE;
        }
    }
    catch (std::runtime_error& e){
        std::cout<<"*** ERROR *** " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...){
        std::cout<<"*** ERROR *** Unknown exception "<< std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}