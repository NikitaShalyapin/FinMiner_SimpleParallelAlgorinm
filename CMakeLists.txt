cmake_minimum_required(VERSION 3.12)
project(FinMiner)

set(CMAKE_CXX_STANDARD 14)

add_executable(FinMiner
        fnv.cpp
        fnv.h
        getNonce.cpp
        getNonce.h
        getTrailingZeroesCount.cpp
        getTrailingZeroesCount.h
        jenkins.cpp
        jenkins.h
        main.cpp
        pjw.cpp
        pjw.h)



find_library(PTHREAD_LIBRARY pthread)
target_link_libraries(FinMiner ${PTHREAD_LIBRARY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp -pthread")

include_directories(/usr/include/mpi/)
set(CMAKE_C_COMPILER /usr/bin/mpicc)
set(CMAKE_CXX_COMPILER /usr/bin/mpic++)

#set(BOOST_ROOT /usr/include/boost/1.68.0)

find_package(Boost 1.68.0 COMPONENTS system filesystem REQUIRED)
if(Boost_FOUND)
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_VERSION: ${Boost_VERSION}")
include_directories(${Boost_INCLUDE_DIRS})
endif()

if(Boost_FOUND)
target_link_libraries(FinMiner ${Boost_LIBRARIES})
endif()