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

add_executable(test-articulation-points executables/test-articulation-points.cpp)
target_sources(test-articulation-points PRIVATE src/mc-graph.cpp)
target_link_libraries(test-articulation-points Threads::Threads)



#add_executable(eval_single-thread tests/test-and-eval-single.cpp)
#target_sources(eval_single-thread PRIVATE implementation/dynamic_connectivity.cpp)
#target_link_libraries(eval_single-thread Threads::Threads)