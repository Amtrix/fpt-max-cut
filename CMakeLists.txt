cmake_minimum_required (VERSION 2.8)

#### USER DEFINED ##############################################################

#### BASIC SETTINGS ############################################################

include_directories(.)

set (CMAKE_CXX_FLAGS "-std=c++1z -msse4.2 -Wall -Wextra -O3 -g -fopenmp")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

#### TARGETS ###################################################################

add_executable(benchmark executables/benchmark.cpp)
target_sources(benchmark PRIVATE src/mc-graph.cpp)
target_link_libraries(benchmark Threads::Threads)

add_executable(test-articulation-and-biconnected executables/test-articulation-and-biconnected.cpp)
target_sources(test-articulation-and-biconnected PRIVATE src/mc-graph.cpp)
target_link_libraries(test-articulation-and-biconnected Threads::Threads)

