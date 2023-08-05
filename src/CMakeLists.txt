cmake_minimum_required(VERSION 3.26)

add_library(
    cip_shuffle INTERFACE
    cip_shuffle.hpp
)

target_include_directories(
    cip_shuffle INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)