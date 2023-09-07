#include <iostream>
#include <chrono>
#include <cip_shuffle.hpp>

int main() {
    int seed = 12345;
    std::mt19937_64 generator(seed);

    constexpr std::size_t num_buckets = 4;
    const std::size_t size = 100000000;
    const std::size_t runs = 10;

    for (std::size_t i = 0; i < runs + 1; i++) {
        std::vector<std::size_t> vec(size);
        std::iota(vec.begin(), vec.end(), 0);
        std::span vector_span {vec};

        auto start = std::chrono::steady_clock::now();
        inplace_scatter_shuffle<num_buckets>(vector_span, generator);
        auto end = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        std::cout << duration.count() << " ns " << "\n";
    }

    return 0;
}
