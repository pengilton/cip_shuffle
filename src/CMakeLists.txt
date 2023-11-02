set(BUILD_SINGLE_EXECUTABLES FALSE)

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

# Builds either one or multiple executables.
if(BUILD_SINGLE_EXECUTABLES)
    # This one builds only one exectuable with the default parameters set in the header file
    add_executable(main main.cpp)
    target_link_libraries(main cip_shuffle pcg_cpp)
else()
    # Here we can build multiple executable so we can run one bash script which runs all exectuables back to back
    add_executable(main-4-8-8 main.cpp)
    target_link_libraries(main-4-8-8 cip_shuffle pcg_cpp)
    target_compile_definitions(main-4-8-8 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=4 
                               -DLOG_BUFFER_SIZE_VAR=8 
                               -DLOG_THRESHOLD_VAR=8)
endif()