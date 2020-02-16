#include <iostream>
#include "ArgParser.h"
#include "TaskQueue.h"
#include "Reader.h"
#include "Writer.h"
#include <filesystem>
#include <deque>
#include <thread>
#include <boost/crc.hpp>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        fs::path inputFilePath = "";
        std::string outputFilePath = "";
        uint32_t sizeBlock = 0;

        /** Parse arguments command lines */
        try {
            ArgParser args(argc, argv);
            inputFilePath = args.getInputFile();
            outputFilePath = args.getOutputFile();
            sizeBlock = args.getSizeBlock();
        }
        catch (std::invalid_argument &e) {
            std::cout << "*** ERROR *** Incorrect arguments. " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        catch (std::exception& e){
            std::cout << "*** ERROR *** Incorrect  " << e.what() << std::endl;
            return EXIT_FAILURE;
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
            auto reader = Reader(inputFilePath,sizeBlock);
            /** Get pointers on the memory`s block*/
            auto pointers = reader.getPointers();
            /** Opens the output file*/
            const size_t outsize = pointers.size() * sizeof(uint32_t);//becose crc32
            auto writer = Writer(outputFilePath, outsize);

            /** If system has a one core */
            hwConcurency = std::max<uint32_t >(hwConcurency,1);

            /** Thread pool */
            TaskQueue queue(hwConcurency);

            int offset = 0;
            for(const auto& source : pointers){
                char* p = writer.get(offset);
                /** offset equal crc32*/
                offset += sizeof(uint32_t);

                /** Lambda is reading data from input file, compute crc32 and write result to buffer*/
                auto processing = [p,source](){
                    boost::crc_32_type crc;
                    crc.process_bytes(source.first, source.second);
                    uint32_t  checksum = crc.checksum();
                    std::memcpy(p,&checksum, sizeof(checksum));
                };

                /** Push tasks to threads` poll */
                queue.push(processing);
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