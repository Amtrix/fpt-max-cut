cmake_minimum_required (VERSION 2.8)

#### USER DEFINED ##############################################################

#### BASIC SETTINGS ############################################################

include_directories(.)

set (CMAKE_CXX_COMPILER /usr/bin/g++-7)
set (CMAKE_CXX_FLAGS "-std=c++1z -msse4.2 -Wall -Wextra -O3 -g -fopenmp")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

#### TARGETS ###################################################################

#ALL_CPP_FILES = src/mc-graph.cpp src/utils.cpp

list(APPEND ALL_CPP_FILES
    "src/mc-graph.cpp"
    "src/utils.cpp"
)

add_executable(benchmark executables/benchmark.cpp)
target_sources(benchmark PRIVATE ${ALL_CPP_FILES})
target_link_libraries(benchmark PRIVATE Threads::Threads -lstdc++fs)

add_executable(benchmark-debug executables/benchmark.cpp)
target_sources(benchmark-debug PRIVATE ${ALL_CPP_FILES})
target_link_libraries(benchmark-debug PRIVATE Threads::Threads -lstdc++fs)
target_compile_definitions(benchmark-debug PRIVATE DEBUG)

add_executable(test-articulation-and-biconnected executables/test-articulation-and-biconnected.cpp)
target_sources(test-articulation-and-biconnected PRIVATE ${ALL_CPP_FILES})
target_link_libraries(test-articulation-and-biconnected Threads::Threads -lstdc++fs)

add_executable(test-graph-functionality executables/test-graph-functionality.cpp)
target_sources(test-graph-functionality PRIVATE ${ALL_CPP_FILES})
target_link_libraries(test-graph-functionality Threads::Threads -lstdc++fs)

add_executable(unweight-an-instance executables/utils/unweight-an-instance.cpp)
target_link_libraries(unweight-an-instance Threads::Threads -lstdc++fs)

add_executable(double-clique-solver executables/double-clique-solver.cpp)
target_sources(double-clique-solver PRIVATE ${ALL_CPP_FILES})
target_link_libraries(double-clique-solver PRIVATE Threads::Threads -lstdc++fs)