#include <iostream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cip_shuffle.hpp>

std::filesystem::path create_csv_path(size_t num_buckets, size_t threshold) {
    // Folder wher all benchmarks should be stored
    std::filesystem::path path = "../benchmarks";

    // Solution based on https://stackoverflow.com/questions/5438482/getting-the-current-time-as-a-yyyy-mm-dd-hh-mm-ss-string
    // We transfomr date and time to YYYYMMDD-HHMMSS
    std::time_t rawtime;
    std::tm* timeinfo;
    char buffer [80];
    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
    std::strftime(buffer,80,"%Y%m%d-%H%M%S", timeinfo);

    std::string filename = std::string(buffer) 
                           + "-nb=" + std::to_string(num_buckets) 
                           + "-th=" + std::to_string(threshold)
                           + ".csv";

    // Creates full path. Should be plattform independent
    path /= filename;

    return path;
}

int main() {
    int seed = 12345;
    std::mt19937_64 generator(seed);

    constexpr std::size_t num_buckets = 4;
    constexpr std::size_t threshold = 256;
    const std::size_t runs = 10;
    const std::size_t max_exp = 16;

    std::filesystem::path path = create_csv_path(num_buckets, threshold);

    std::fstream my_file;
    my_file.open(path, std::ios::out);
    if (my_file.is_open()) {
        std::cout << "Starting benchmark with " << num_buckets << " buckets...\n";

        // Creating CSV headers
        my_file << "buckets," << "threshold," << "run," << "integers," << "duration [ns]" << "\n";

        for (std::size_t i = 0; i < max_exp; i++) {
            std::size_t size = std::pow(2, i);
            std::cout << "Setting size to = " << size << "\n";

            for (std::size_t j = 0; j < runs + 1; j++) {
                std::vector<std::size_t> vec(size);
                std::iota(vec.begin(), vec.end(), 0);
                std::span vector_span {vec};

                auto start = std::chrono::steady_clock::now();
                inplace_scatter_shuffle<num_buckets>(vector_span, generator);
                auto end = std::chrono::steady_clock::now();

                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

                // We skip the first run as a warmup.
                if (j == 0) {
                    continue;
                }

                // std::cout << "Duration of run " << std::setw(2) << j << ": " << std::setw(18) << duration.count() << " ns" << "\n";
                my_file << num_buckets << ",";
                my_file << threshold << ",";
                my_file << j << ",";
                my_file << size << ",";
                my_file << duration.count() << "\n";
            }
        }

        std::cout << "Benchmark done!" << std::endl;

        my_file.close();
    } else {
        std::cout << "ERROR: File not found!" << "\n";
    }

    return 0;
}
