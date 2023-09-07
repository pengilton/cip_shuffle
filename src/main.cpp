#include <iostream>
#include <iomanip>
#include <chrono>
#include <cip_shuffle.hpp>

int main() {
    int seed = 12345;
    std::mt19937_64 generator(seed);

    constexpr std::size_t num_buckets = 4;
    const std::size_t runs = 10;
    const std::size_t max_exp = 2;

    std::cout << "Starting benchmark with " << num_buckets << " buckets...\n";

    for (std::size_t i = 0; i < max_exp; i++) {
        std::size_t size = std::pow(2, i);
        std::cout << "\n" << "Setting size to = " << size << "\n";

        for (std::size_t j = 0; j < runs + 1; j++) {
            std::vector<std::size_t> vec(size);
            std::iota(vec.begin(), vec.end(), 0);
            std::span vector_span {vec};

            auto start = std::chrono::steady_clock::now();
            inplace_scatter_shuffle<num_buckets>(vector_span, generator);
            auto end = std::chrono::steady_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

            // We skip the first run as warmup.
            if (j == 0) {
                continue;
            }

            std::cout << "Duration of run " << std::setw(2) << j << ": " << std::setw(18) << duration.count() << " ns" << "\n";
        }
    }

    return 0;
}
