add_library(
    cip_shuffle INTERFACE
    cip_shuffle.hpp
)
target_include_directories(
    cip_shuffle INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# adding pcg
add_library(
    pcg_cpp INTERFACE
)
target_include_directories(
    pcg_cpp INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/pcg-cpp-0.98/include"
)

add_executable(main main.cpp)
target_link_libraries(main cip_shuffle pcg_cpp)
target_compile_definitions(main PUBLIC -DLOG_NUM_BUCKETS_VAR=4 -DLOG_THRESHOLD_VAR=8)