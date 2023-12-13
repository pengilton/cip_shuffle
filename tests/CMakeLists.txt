add_executable(chi_squared_test chi_squared_test.cpp)
# add_executable(distribution_tests distribution_tests.cpp)

find_package(Boost 1.74)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# adding pcg
target_include_directories(
    pcg_cpp INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/pcg-cpp-0.98/include"
)

target_link_libraries(
    chi_squared_test
    GTest::gtest_main
    cip_shuffle
    pcg_cpp
)

include(GoogleTest)
gtest_discover_tests(chi_squared_test)