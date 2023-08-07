cmake_minimum_required(VERSION 3.26)

add_executable(cip_shuffle_test cip_shuffle_test.cpp)

find_package(Boost 1.82 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

target_link_libraries(
    cip_shuffle_test
    GTest::gtest_main
    cip_shuffle
)

include(GoogleTest)
gtest_discover_tests(cip_shuffle_test)