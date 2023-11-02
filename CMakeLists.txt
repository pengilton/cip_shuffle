cmake_minimum_required(VERSION 3.26)

project(cip_shuffle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(ENABLE_TESTS TRUE)

if(CMAKE_C_COMPILER_ID)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()
if(MSVC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

# Add sanitizer flags for C++ compiler for "Debug" configuration
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined")

if(ENABLE_TESTS)
    # Adding Google Test. As in http://google.github.io/googletest/quickstart-cmake.html
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/40412d85124f7c6f3d88454583c4633e5e10fc8c.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
endif()

add_subdirectory(src)

if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()
