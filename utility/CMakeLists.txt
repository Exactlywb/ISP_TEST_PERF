cmake_minimum_required (VERSION 3.12)
project (c3_utility)

set (SOURCES
    source/main.cpp
    source/algorithm.cpp
    source/perfParser.cpp
)


find_package (Boost 1.60 COMPONENTS filesystem REQUIRED)

add_executable (${PROJECT_NAME} ${SOURCES})
target_compile_features (${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include/")

target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES})
