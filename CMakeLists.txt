cmake_minimum_required (VERSION 2.8)

include(JSONParser.cmake)

#### USER DEFINED ##############################################################
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/build-config.json configJsonFile)
sbeParseJson(configJson configJsonFile)

foreach(var ${configJson})
    message("${var} = ${${var}}")
endforeach()

set(LOCALSOLVER_ENABLED ${configJson.localsolver.enabled})
set(LOCALSOLVER_USE_CONCURRENCY ${configJson.localsolver.use-parallelism})
set(LOCALSOLVER_BIN_PATH ${configJson.localsolver.bin-path})
set(BIQMAC_ENABLED ${configJson.biqmac.enabled})
set(BIQMAC_BINARY_PATH ${configJson.biqmac.binary-file})
set(BIQMAC_BINARY_PATH_EVAL ${CMAKE_CURRENT_SOURCE_DIR}/../BiqMac/bab)
set(MQLIB_DATA ${configJson.mqlib-set})

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${BIQMAC_BINARY_PATH})
    set(BIQMAC_BINARY_PATH_EVAL ${CMAKE_CURRENT_SOURCE_DIR}/${BIQMAC_BINARY_PATH})
endif()

if (${LOCALSOLVER_USE_CONCURRENCY})
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/scripts/experiments/cmake.config.generated.sh "config_use_full_concurrency=true")
endif()

message("")

#### BASIC SETTINGS ############################################################

add_subdirectory(./test-generators/KaGen/)
include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/solvers/MQLib/include)
include_directories(. /software/localsolver/localsolver_8_0/include)
include_directories(. /opt/localsolver_7_5/include)
include_directories(. /opt/localsolver_8_5/include)

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
find_library(LOCALSOLVER localsolver PATHS /software/localsolver/localsolver_8_0/bin /opt/localsolver_7_5/bin /opt/localsolver_8_5/bin ${LOCALSOLVER_BIN_PATH})


set(TARGET_LIBS Threads::Threads -lstdc++fs ${LOCALSOLVER} ${MQLIB} generators)

macro(add_binary_exec TARGETNAME DEBUGMODE ENTRYCPP)
    add_executable(${TARGETNAME} ${ENTRYCPP})
    target_sources(${TARGETNAME} PRIVATE ${ALL_CPP_FILES})
    target_link_libraries(${TARGETNAME} PRIVATE ${TARGET_LIBS})
    target_compile_definitions(${TARGETNAME} PRIVATE ${DEBUGMODE} ${LOCALSOLVER_FLAG})

    target_compile_definitions(${TARGETNAME} PRIVATE PROJECT_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")

    if (${LOCALSOLVER_ENABLED})
        target_compile_definitions(${TARGETNAME} PRIVATE LOCALSOLVER_EXISTS=1)

        if (${LOCALSOLVER_USE_CONCURRENCY})
            target_compile_definitions(${TARGETNAME} PRIVATE LOCALSOLVER_USE_CONCURRENCY=1)
        endif()
    endif()
    
    IF(${BIQMAC_ENABLED} AND EXISTS ${BIQMAC_BINARY_PATH_EVAL})
        target_compile_definitions(${TARGETNAME} PRIVATE BIQMAC_BINARY_PATH="${BIQMAC_BINARY_PATH_EVAL}")
        target_compile_definitions(${TARGETNAME} PRIVATE BIQMAC_EXISTS=1)
    endif()

    target_compile_definitions(${TARGETNAME} PRIVATE BENCHMARK_NUMBER_OF_THREADS=1)
    target_compile_definitions(${TARGETNAME} PRIVATE MQLIB_DATAPATH="${MQLIB_DATA}")
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
add_binary_exec(playground                           DEBUG   executables/playground.cpp)
add_binary_exec(find-kernelization-grid              NDEBUG  executables/find-kernelization-grid.cpp)