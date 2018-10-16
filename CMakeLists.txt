cmake_minimum_required (VERSION 2.8)

#### USER DEFINED ##############################################################

#### BASIC SETTINGS ############################################################

include_directories(. ./solvers/MQLib/include)

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
message(${CMAKE_FIND_ROOT_PATH}/solvers/MQLib/bin/)

find_library(MQLIB NAMES "MQLib.a" PATHS ./solvers/MQLib/bin/)

list(APPEND TARGET_LIBS
    "Threads::Threads"
    "-lstdc++fs"
    ${MQLIB}
)

#list(APPEND OUTPUT_NAME "benchmark-kernelization-by-clique")
#list(APPEND FLAGS "BENCH_KERNELIZATION_CLIQUE")

add_executable(benchmark executables/benchmark.cpp)
target_sources(benchmark PRIVATE ${ALL_CPP_FILES})
target_link_libraries(benchmark PRIVATE ${TARGET_LIBS})
target_compile_definitions(benchmark PRIVATE NDEBUG)

add_executable(benchmark-debug executables/benchmark.cpp)
target_sources(benchmark-debug PRIVATE ${ALL_CPP_FILES})
target_link_libraries(benchmark-debug PRIVATE ${TARGET_LIBS})
target_compile_definitions(benchmark-debug PRIVATE DEBUG)

add_executable(test-articulation-and-biconnected executables/test-articulation-and-biconnected.cpp)
target_sources(test-articulation-and-biconnected PRIVATE ${ALL_CPP_FILES})
target_link_libraries(test-articulation-and-biconnected ${TARGET_LIBS})

add_executable(test-graph-functionality executables/test-graph-functionality.cpp)
target_sources(test-graph-functionality PRIVATE ${ALL_CPP_FILES})
target_link_libraries(test-graph-functionality ${TARGET_LIBS})

add_executable(test-kernelization-all executables/test-kernelization-all.cpp)
target_sources(test-kernelization-all PRIVATE ${ALL_CPP_FILES})
target_link_libraries(test-kernelization-all ${TARGET_LIBS})

add_executable(test-kernelization-auto executables/test-kernelization-auto.cpp)
target_sources(test-kernelization-auto PRIVATE ${ALL_CPP_FILES})
target_link_libraries(test-kernelization-auto ${TARGET_LIBS})

add_executable(unweight-an-instance executables/utils/unweight-an-instance.cpp)
target_link_libraries(unweight-an-instance ${TARGET_LIBS})

add_executable(double-clique-solver executables/double-clique-solver.cpp)
target_sources(double-clique-solver PRIVATE ${ALL_CPP_FILES})
target_link_libraries(double-clique-solver PRIVATE ${TARGET_LIBS})

add_executable(random-playground executables/random-playground.cpp)
target_sources(random-playground PRIVATE ${ALL_CPP_FILES})
target_link_libraries(random-playground PRIVATE ${TARGET_LIBS})

add_executable(find-kernelization-general executables/find-kernelization-general.cpp)
target_sources(find-kernelization-general PRIVATE ${ALL_CPP_FILES})
target_link_libraries(find-kernelization-general PRIVATE ${TARGET_LIBS})

add_executable(find-kernelization-weighted executables/find-kernelization-weighted.cpp)
target_sources(find-kernelization-weighted PRIVATE ${ALL_CPP_FILES})
target_link_libraries(find-kernelization-weighted PRIVATE ${TARGET_LIBS})



add_executable(find-kernelization-clique executables/find-kernelization-clique.cpp)
target_sources(find-kernelization-clique PRIVATE ${ALL_CPP_FILES})
target_link_libraries(find-kernelization-clique PRIVATE ${TARGET_LIBS})

add_executable(lower-bound-num-of-class-computation executables/lower-bound-num-of-class-computation.cpp)
target_sources(lower-bound-num-of-class-computation PRIVATE ${ALL_CPP_FILES})
target_link_libraries(lower-bound-num-of-class-computation PRIVATE ${TARGET_LIBS})


add_executable(remove-double-edges executables/remove-double-edges.cpp)
target_sources(remove-double-edges PRIVATE ${ALL_CPP_FILES})
target_link_libraries(remove-double-edges PRIVATE ${TARGET_LIBS})