cmake_minimum_required (VERSION 3.12)
project (c3_utility)

set (SOURCES
    source/main.cpp
    source/Algorithm.cpp
    source/PerfParser.cpp
    source/NMParser.cpp
    source/Subprocess.cpp
    source/FunctionData.cpp

    source/algos/Annealing.cpp
    source/algos/QAP.cpp
    source/algos/Percise.cpp

    source/Metrics.cpp
)

set (COMPILE_SETTINGS -O0 -g -Wall -Wextra -Wpedantic)

find_package (Boost 1.60 COMPONENTS program_options filesystem REQUIRED)

add_executable (${PROJECT_NAME} ${SOURCES})
target_compile_features (${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include/")

target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES})

target_compile_options (${PROJECT_NAME} PRIVATE ${COMPILE_SETTINGS})

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/*.[ch]pp
        ${PROJECT_SOURCE_DIR}/*.[ch]
    )

    add_custom_target(
        format
        COMMAND clang-format
        -i
        -style=file
        ${ALL_CXX_SOURCE_FILES}
    )
endif()
