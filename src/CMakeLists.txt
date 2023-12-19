# 0 = single exec, main.cpp
# 1 = base_case_bench
# 2 = buffer test
# 3 = buckets
# 4 = threshold
# 5 = treshold fy32
set(BUILD_EXECUTABLES "3")

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
if(BUILD_EXECUTABLES EQUAL "0")
    # This one builds only one exectuable with the default parameters set in the header file
    add_executable(main main.cpp)
    target_link_libraries(main cip_shuffle pcg_cpp)
elseif(BUILD_EXECUTABLES EQUAL "1")
    add_executable(base_case_bench base_case_bench.cpp)
    target_link_libraries(base_case_bench cip_shuffle pcg_cpp)
elseif(BUILD_EXECUTABLES EQUAL "2")
    add_executable(buffered_bench_0 buffered_bench.cpp)
    target_link_libraries(buffered_bench_0 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_0 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=0)

    add_executable(buffered_bench_1 buffered_bench.cpp)
    target_link_libraries(buffered_bench_1 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_1 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=1)
    
    add_executable(buffered_bench_2 buffered_bench.cpp)
    target_link_libraries(buffered_bench_2 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_2 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=2)

    add_executable(buffered_bench_3 buffered_bench.cpp)
    target_link_libraries(buffered_bench_3 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_3 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=3)
    
    add_executable(buffered_bench_4 buffered_bench.cpp)
    target_link_libraries(buffered_bench_4 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_4 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=4)
    
    add_executable(buffered_bench_5 buffered_bench.cpp)
    target_link_libraries(buffered_bench_5 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_5 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=5)
    
    add_executable(buffered_bench_6 buffered_bench.cpp)
    target_link_libraries(buffered_bench_6 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_6 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=6)

    add_executable(buffered_bench_7 buffered_bench.cpp)
    target_link_libraries(buffered_bench_7 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_7 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=7)

    add_executable(buffered_bench_8 buffered_bench.cpp)
    target_link_libraries(buffered_bench_8 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_8 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=8)
    
    add_executable(buffered_bench_9 buffered_bench.cpp)
    target_link_libraries(buffered_bench_9 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_9 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=9)

    add_executable(buffered_bench_10 buffered_bench.cpp)
    target_link_libraries(buffered_bench_10 cip_shuffle pcg_cpp)
    target_compile_definitions(buffered_bench_10 PUBLIC 
                               -DLOG_BUFFER_SIZE_VAR=10)
elseif(BUILD_EXECUTABLES EQUAL "3")
    add_executable(cip_shuffle_bench_1 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_1 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_1 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=1)
    
    add_executable(cip_shuffle_bench_2 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_2 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_2 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=2)
    
    add_executable(cip_shuffle_bench_3 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_3 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_3 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=3)

    add_executable(cip_shuffle_bench_4 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_4 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_4 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=4)

    add_executable(cip_shuffle_bench_5 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_5 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_5 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=5)
    
    add_executable(cip_shuffle_bench_6 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_6 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_6 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=6)
    
    add_executable(cip_shuffle_bench_7 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_7 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_7 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=7)
    
    add_executable(cip_shuffle_bench_8 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_8 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_8 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=8)
    
    add_executable(cip_shuffle_bench_9 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_9 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_9 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=9)
    
    add_executable(cip_shuffle_bench_10 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_10 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_10 PUBLIC 
                               -DLOG_NUM_BUCKETS_VAR=10)
elseif(BUILD_EXECUTABLES EQUAL "4")
    add_executable(cip_shuffle_bench_10 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_10 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_10 PUBLIC 
                               -DLOG_THRESHOLD_VAR=10)

    add_executable(cip_shuffle_bench_11 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_11 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_11 PUBLIC 
                               -DLOG_THRESHOLD_VAR=11)
    
    add_executable(cip_shuffle_bench_12 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_12 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_12 PUBLIC 
                               -DLOG_THRESHOLD_VAR=12)

    add_executable(cip_shuffle_bench_13 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_13 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_13 PUBLIC 
                               -DLOG_THRESHOLD_VAR=13)

    add_executable(cip_shuffle_bench_14 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_14 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_14 PUBLIC 
                               -DLOG_THRESHOLD_VAR=14)

    add_executable(cip_shuffle_bench_15 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_15 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_15 PUBLIC 
                               -DLOG_THRESHOLD_VAR=15)
    
    add_executable(cip_shuffle_bench_16 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_16 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_16 PUBLIC 
                               -DLOG_THRESHOLD_VAR=16)

    add_executable(cip_shuffle_bench_17 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_17 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_17 PUBLIC 
                               -DLOG_THRESHOLD_VAR=17)
    
    add_executable(cip_shuffle_bench_18 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_18 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_18 PUBLIC 
                               -DLOG_THRESHOLD_VAR=18)

    add_executable(cip_shuffle_bench_19 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_19 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_19 PUBLIC 
                               -DLOG_THRESHOLD_VAR=19)
    
    add_executable(cip_shuffle_bench_20 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_20 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_20 PUBLIC 
                               -DLOG_THRESHOLD_VAR=20)

    add_executable(cip_shuffle_bench_21 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_21 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_21 PUBLIC 
                               -DLOG_THRESHOLD_VAR=21)

    add_executable(cip_shuffle_bench_22 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_22 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_22 PUBLIC 
                               -DLOG_THRESHOLD_VAR=22)
elseif(BUILD_EXECUTABLES EQUAL "5")    
    add_executable(cip_shuffle_bench_18 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_18 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_18 PUBLIC 
                               -DLOG_THRESHOLD_VAR=18)

    add_executable(cip_shuffle_bench_19 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_19 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_19 PUBLIC 
                               -DLOG_THRESHOLD_VAR=19)
    
    add_executable(cip_shuffle_bench_20 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_20 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_20 PUBLIC 
                               -DLOG_THRESHOLD_VAR=20)

    add_executable(cip_shuffle_bench_21 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_21 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_21 PUBLIC 
                               -DLOG_THRESHOLD_VAR=21)

    add_executable(cip_shuffle_bench_22 cip_shuffle_bench.cpp)
    target_link_libraries(cip_shuffle_bench_22 cip_shuffle pcg_cpp)
    target_compile_definitions(cip_shuffle_bench_22 PUBLIC 
                               -DLOG_THRESHOLD_VAR=22)
endif()