#include <iostream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cip_shuffle.hpp>

#include "pcg-cpp-0.98/include/pcg_random.hpp"

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

//----------------------------------------------------------------------------------------------------------------
struct benchmark_param {
    std::string function_name;
    std::string prng_name;
    std::size_t num_buckets;
    std::size_t buffer_size;
    std::size_t threshold;
    std::size_t min_exp;
    std::size_t max_exp;
    std::size_t default_runs;
    std::chrono::milliseconds min_duration;
};

void benchmark_inplace_scatter_shuffle_pcg() {
    benchmark_param benchmark;
    benchmark.function_name = "inplace_scatter_shuffle";
    benchmark.prng_name = "pcg64";
    benchmark.num_buckets = NUM_BUCKETS;
    benchmark.buffer_size = BUFFER_SIZE;
    benchmark.threshold = THRESHOLD;
    benchmark.min_exp = 0;
    benchmark.max_exp = 29;
    benchmark.default_runs = 10;
    benchmark.min_duration = std::chrono::milliseconds(100);

    pcg_extras::seed_seq_from<std::random_device> seed_source;
    pcg64 generator(seed_source);

    std::filesystem::path path = create_csv_path(benchmark.num_buckets, benchmark.buffer_size, benchmark.threshold);

    std::fstream my_file;
    my_file.open(path, std::ios::out);
    if (my_file.is_open()) {
        std::cout << "Starting benchmark with " << benchmark.num_buckets << " buckets...\n";

        // Creating CSV headers
        my_file << "function," << "prng," << "buckets," << "buffer," << "threshold," 
                << "min_exp," << "max_exp,"
                << "integers," << "total_runs," << "total_runtime" << "\n";

        // Initiliazing vector with maximum size
        std::vector<std::size_t> vec(std::pow(2, benchmark.max_exp));
        std::iota(vec.begin(), vec.end(), 0);
        std::span vector_span {vec};

        for (std::size_t i = benchmark.min_exp; i <= benchmark.max_exp; i++) {
            std::size_t size = std::pow(2, i);
            std::cout << std::setw(static_cast<size_t>(std::log10(benchmark.max_exp))) << i + 1 << "/" << benchmark.max_exp + 1 << " ";
            std::cout << "Setting size = " << std::setw(static_cast<size_t>(std::log10(std::pow(2, benchmark.max_exp)))) << size;
            std::cout << " " << "which needs " << sizeof(size_t) * size << " Bytes of storage.\n";

            // Getting the first size elements
            std::span view = vector_span.first(size);

            std::size_t total_runs = benchmark.default_runs;
            while (true) {
                auto start = std::chrono::steady_clock::now();
                for (std::size_t i = 0; i < total_runs; i++) {
                    inplace_scatter_shuffle(view, generator);
                }
                auto end = std::chrono::steady_clock::now();

                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                if (duration >= static_cast<std::chrono::nanoseconds>(benchmark.min_duration)) {
                    my_file << benchmark.function_name << ",";
                    my_file << benchmark.prng_name << ",";
                    my_file << benchmark.num_buckets << ",";
                    my_file << benchmark.buffer_size << ",";
                    my_file << benchmark.threshold << ",";
                    my_file << benchmark.min_exp << ",";
                    my_file << benchmark.max_exp << ",";
                    my_file << size << ",";
                    my_file << total_runs << ",";
                    my_file << duration.count() << "\n";
                    std::cout << "Total runtime: " << std::setw(18) << duration.count() << " ns" << "\n";
                    break;
                }
                total_runs *= 10;
            }
            std::cout << "\n";
        }

        std::cout << "Benchmark done!" << std::endl;

        my_file.close();
    } else {
        std::cout << "ERROR: File not found!" << "\n";
    }
} 

//----------------------------------------------------------------------------------------------------------------

int main() {
    benchmark_inplace_scatter_shuffle_pcg();
    return 0;
}
