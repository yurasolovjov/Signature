#include <iostream>
#include "ArgParser.h"
#include <thread>
#include <filesystem>
#include "TaskQueue.h"
#include <deque>
#include <boost/crc.hpp>

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

            /** Opens the output file*/
            auto output = std::make_unique<std::ofstream>(outputFilePath, std::ios::binary | std::ios_base::out);

            if (!output) {
                throw std::runtime_error("Output file is not available");
            }

            /** If system has a one core */
            hwConcurency = std::max<uint32_t >(hwConcurency,1);

            /** Thread pool */
            TaskQueue queue(hwConcurency);

            /** Mutex for read operation with file*/
            std::mutex mutexRead;
            /** Mutex for operation with stack*/
            std::mutex mutexWrite;
            /** Mutex for write operation with file*/
            std::mutex mutexFile;
            /** Middle buffer for output*/
            std::vector<uint32_t> outBuffer;

            while (inputFile->good()) {

                /** Lambda is reading data from input file, compute crc32 and write result to middle buffer*/
                auto processing = [&mutexRead,&mutexWrite,&inputFile,&queue,&sizeBlock,&outBuffer,&output,&mutexFile](){

                    auto buffer = std::unique_ptr<char[]>(new char[sizeBlock]);

                    size_t gcount = 0;

                    {
                        /** Read from file to buffer */
                        std::lock_guard _lock(mutexRead);

                        if(!inputFile->good()){
                            return;
                        }

                        inputFile->read(buffer.get(), sizeBlock);
                        gcount = inputFile->gcount();
                    }

                    /** Mutex is free*/
                    boost::crc_32_type crc;
                    crc.process_bytes(buffer.get(), gcount);

                    {
                        std::lock_guard _lock(mutexWrite);
                        outBuffer.push_back(crc.checksum());

                    }
                };

                /** Lambda is writing result from middle buffer to output file */
                auto writeToFile = [&mutexWrite,&outBuffer,&output,&mutexFile](){

                    std::lock_guard _lock(mutexWrite);

                    if(outBuffer.empty()){
                        return;
                    }

                    {
                        std::lock_guard _lockFile(mutexFile);
                        output->write(reinterpret_cast<char *>(outBuffer.data()),
                                      outBuffer.size() * sizeof(uint32_t));

                    }

                    /** Clear data from buffer*/
                    outBuffer.clear();
                };


                /** Push tasks to threads` poll */
                queue.push(processing);
                queue.push(writeToFile);

                /** Check internal exception */
                queue.checkException();
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