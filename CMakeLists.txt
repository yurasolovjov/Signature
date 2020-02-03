cmake_minimum_required(VERSION 3.12)
project(signature)

set(CMAKE_CXX_STANDARD 17)

set(BOOST_DIR ${BOOST_ROOT})
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_DEBUG ON)

find_package(Boost 1.59.0 REQUIRED program_options system COMPONENTS)

set( CMAKE_INCLUDE_CURRENT_DIR ON )

include_directories( ${CMAKE_SOURCE_DIR}/src )

set(SOURCE_FILES

        src/main.cpp
        src/TaskQueue.cpp
        src/TaskQueue.h
        src/ArgParser.cpp
        src/ArgParser.h src/Writer.cpp src/Writer.h)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
#    link_directories(${Boost_LIBRARY_DIRS})

    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
    target_link_libraries( ${PROJECT_NAME} ${Boost_LIBRARIES} stdc++fs pthread)

endif()
