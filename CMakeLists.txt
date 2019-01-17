cmake_minimum_required (VERSION 2.8)

#### USER DEFINED ##############################################################


#### BASIC SETTINGS ############################################################

#set(CMAKE_BUILD_TYPE "Debug")

message("Add KaGen")
add_subdirectory(./test-generators/KaGen/)
message("Include MQLIB includes")
include_directories(. ./solvers/MQLib/include)
include_directories(. /software/localsolver/localsolver_8_0/include)
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
    "src/output-filter.cpp"
)

find_library(MQLIB NAMES "MQLib.a" PATHS ./solvers/MQLib/bin/)
find_library(LOCALSOLVER localsolver PATHS /software/localsolver/localsolver_8_0/bin /opt/localsolver_7_5/bin)

set(TARGET_LIBS Threads::Threads -lstdc++fs ${LOCALSOLVER} ${MQLIB} generators)

set(LOCALSOLVER_LICENSE_EXISTS FALSE)
set(BIQMAC_EXISTS FALSE)

if (EXISTS /opt/localsolver_7_5/license.dat OR EXISTS /software/localsolver/localsolver_8_0/license.dat)
    message("LOCALSOLVER is ADDED.")
    set(LOCALSOLVER_LICENSE_EXISTS TRUE)
endif()

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../BiqMac)
    message("BIQMAC is ADDED.")
    set(BIQMAC_EXISTS TRUE)
endif()

message("Localsolver license exists flag: ${LOCALSOLVER_LICENSE_EXISTS}")

macro(add_binary_exec TARGETNAME DEBUGMODE ENTRYCPP)
    add_executable(${TARGETNAME} ${ENTRYCPP})
    target_sources(${TARGETNAME} PRIVATE ${ALL_CPP_FILES})
    target_link_libraries(${TARGETNAME} PRIVATE ${TARGET_LIBS})
    target_compile_definitions(${TARGETNAME} PRIVATE ${DEBUGMODE} ${LOCALSOLVER_FLAG})

    target_compile_definitions(${TARGETNAME} PRIVATE PROJECT_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")

    if (${LOCALSOLVER_LICENSE_EXISTS})
        target_compile_definitions(${TARGETNAME} PRIVATE LOCALSOLVER_EXISTS=1)
    endif()

    IF(${BIQMAC_EXISTS})
        target_compile_definitions(${TARGETNAME} PRIVATE BIQMAC_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../BiqMac")
        target_compile_definitions(${TARGETNAME} PRIVATE BIQMAC_EXISTS=1)
    endif()
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
add_binary_exec(find-kernelization-general-debug     DEBUG   executables/find-kernelization-general.cpp)
add_binary_exec(find-kernelization-weighted          NDEBUG  executables/find-kernelization-weighted.cpp)
add_binary_exec(lower-bound-num-of-class-computation NDEBUG  executables/lower-bound-num-of-class-computation.cpp)
add_binary_exec(remove-double-edges                  NDEBUG  executables/remove-double-edges.cpp)
add_binary_exec(playground                           DEBUG  executables/playground.cpp)