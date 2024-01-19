C++ In-Place Shuffle

The goal of this repository is to implement the in-place shuffle algorithm in C++ which is mentioned here https://crates.io/crates/rip_shuffle

Right now, we only have a sequential version of this and it is not quite as fast as the sequential version in Rust. The Rust variant is nearly twice as fast. We still think that there is some room for optimization. We would like to implement a parallel version as well. 

The repository is structured as followe:
- benchmakrs inlcude tons of csv files with data and some Python (Juypter Notebook) code to evaluate the data.
- graphs includes some matplotlib graphs
- rust includes the Rust crate mentioend above. We use this to benchmark the Rust code.
- src inlcudes all C++ source files. The whole source code of cip_shuffle is in cip_shuffle.hpp. Note that we also have some benchmark code. For the tests and benchmarks we use the pcg64 PRNG hence we copied the necessarily fiels into the folder pcg-cpp_0.98
- tests includes some statistical tests. The most important one is the chi_squared_test.cpp one.

We use CMake to build all the C++ files.


To use our C++ implementation it is required to use the C++20 standard or higher though we only tested the code with C++20. Just include the cip_shuffle.hpp to your project. Then just call inplace_scatter_shuffle(data_span, &gen). Note that you need to create a std::span of your data. It is recommended to use a PRNG which can generate 64-bit words.

