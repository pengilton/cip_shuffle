#include <iostream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cip_shuffle.hpp>

std::filesystem::path create_csv_path(size_t num_buckets, size_t buffer_size, size_t threshold) {
    // Folder wher all benchmarks should be stored
    std::filesystem::path path = "../benchmarks/cpp";

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
                           + "-bf=" + std::to_string(buffer_size) 
                           + "-th=" + std::to_string(threshold)
                           + "-cpp"
                           + ".csv";

    // Creates full path. Should be plattform independent
    path /= filename;

    return path;
}

int main() {
    int seed = 12345;
    std::mt19937_64 generator(seed);

    const std::size_t runs = 10;
    const std::size_t min_exp = 0;  //inclusive
    const std::size_t max_exp = 30; //exclusive

    std::filesystem::path path = create_csv_path(NUM_BUCKETS, BUFFER_SIZE, THRESHOLD);

    std::fstream my_file;
    my_file.open(path, std::ios::out);
    if (my_file.is_open()) {
        std::cout << "Starting benchmark with " << NUM_BUCKETS << " buckets...\n";

        // Creating CSV headers
        my_file << "buckets," << "threshold," << "run," << "integers," << "duration [ns]" << "\n";

        for (std::size_t i = min_exp; i < max_exp; i++) {
            std::size_t size = std::pow(2, i);
            std::cout << std::setw(static_cast<size_t>(std::log10(max_exp))) << i + 1 << "/" << max_exp << " ";
            std::cout << "Setting size = " << std::setw(static_cast<size_t>(std::log10(std::pow(2, max_exp)))) << size;
            std::cout << " " << "which needs " << sizeof(size_t) * size << " Bytes of storage.\n";

            for (std::size_t j = 0; j < runs + 1; j++) {
                // auto start_setup = std::chrono::steady_clock::now();
                std::vector<std::size_t> vec(size);
                std::iota(vec.begin(), vec.end(), 0);
                std::span vector_span {vec};
                // auto end_setup = std::chrono::steady_clock::now();
                // auto setup_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_setup - start_setup);

                auto start = std::chrono::steady_clock::now();
                inplace_scatter_shuffle(vector_span, generator);
                auto end = std::chrono::steady_clock::now();

                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

                // We skip the first run as a warmup.
                if (j == 0) {
                    continue;
                }

                my_file << NUM_BUCKETS << ",";
                my_file << THRESHOLD << ",";
                my_file << j << ",";
                my_file << size << ",";
                my_file << duration.count() << "\n";

                std::cout << std::setw(2) << j << "/" << runs << " ";
                // std::cout << "Setup:   " << std::setw(18) << setup_duration.count() << " ns" << "\n";
                // std::cout << std::setw(6) << " ";
                std::cout << "Runtime: " << std::setw(18) << duration.count() << " ns" << "\n";
            }

            std::cout << "\n";
        }

        std::cout << "Benchmark done!" << std::endl;

        my_file.close();
    } else {
        std::cout << "ERROR: File not found!" << "\n";
    }

    return 0;
}
