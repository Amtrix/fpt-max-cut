cmake_minimum_required (VERSION 2.8)

#### USER DEFINED ##############################################################


#### BASIC SETTINGS ############################################################

#set(CMAKE_BUILD_TYPE "Debug")

message("Add KaGen")
add_subdirectory(./test-generators/KaGen/)
message("Include MQLIB includes")
include_directories(. ./solvers/MQLib/include)
include_directories(. /opt/localsolver_7_5/include)

set (CMAKE_CXX_COMPILER /usr/bin/g++-7)
set (CMAKE_CXX_FLAGS "-std=c++1z -msse4.2 -Wall -Wextra -O3 -g -fopenmp")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

message("BUILD TYPE: ${CMAKE_BUILD_TYPE}")

#### TARGETS ###################################################################

list(APPEND ALL_CPP_FILES
    "src/mc-graph.cpp"
    "src/utils.cpp"
    "src/graph-database.cpp"
)

find_library(MQLIB NAMES "MQLib.a" PATHS ./solvers/MQLib/bin/)

set(TARGET_LIBS Threads::Threads -lstdc++fs -llocalsolver ${MQLIB} generators)


macro(add_binary_exec TARGETNAME DEBUGMODE ENTRYCPP)
    add_executable(${TARGETNAME} ${ENTRYCPP})
    target_sources(${TARGETNAME} PRIVATE ${ALL_CPP_FILES})
    target_link_libraries(${TARGETNAME} PRIVATE ${TARGET_LIBS})
    target_compile_definitions(${TARGETNAME} PRIVATE ${DEBUGMODE} LOCALSOLVER_EXISTS=1)
endmacro(add_binary_exec)

add_binary_exec(benchmark                            NDEBUG  executables/benchmark.cpp)
add_binary_exec(benchmark-debug                      DEBUG   executables/benchmark.cpp)
add_binary_exec(test-articulation-and-biconnected    DEBUG   executables/test-articulation-and-biconnected.cpp)
add_binary_exec(test-graph-functionality             DEBUG   executables/test-graph-functionality.cpp)
add_binary_exec(test-kernelization-all               DEBUG   executables/test-kernelization-all.cpp)
add_binary_exec(test-kernelization-auto              DEBUG   executables/test-kernelization-auto.cpp)
add_binary_exec(unweight-an-instance                 NDEBUG  executables/utils/unweight-an-instance.cpp)
add_binary_exec(double-clique-solver                 NDEBUG  executables/double-clique-solver.cpp)
add_binary_exec(random-playground                    DEBUG   executables/random-playground.cpp)
add_binary_exec(find-kernelization-general           NDEBUG  executables/find-kernelization-general.cpp)
add_binary_exec(find-kernelization-weighted          NDEBUG  executables/find-kernelization-weighted.cpp)
add_binary_exec(lower-bound-num-of-class-computation NDEBUG  executables/lower-bound-num-of-class-computation.cpp)
add_binary_exec(remove-double-edges                  NDEBUG  executables/remove-double-edges.cpp)