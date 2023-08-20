cmake_minimum_required(VERSION 3.26)

add_executable(chi_squared_test chi_squared_test.cpp)

find_package(Boost 1.82 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

target_link_libraries(
    chi_squared_test
    GTest::gtest_main
    cip_shuffle
)

include(GoogleTest)
gtest_discover_tests(chi_squared_test)