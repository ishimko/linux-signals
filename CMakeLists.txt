cmake_minimum_required(VERSION 3.4)
project(linux_signals)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.c utils.h utils.c config.c config.h)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(linux_signals ${SOURCE_FILES})