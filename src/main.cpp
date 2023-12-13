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
    std::size_t size;
    std::size_t total_runs;
    std::chrono::nanoseconds total_runtime;
    std::size_t DEFAULT_RUNS;
    std::chrono::milliseconds MIN_DURATION;

    void create_header(std::fstream& file) {
        // Creating CSV headers
        file << "function," << "prng," << "buckets," << "buffer," << "threshold," 
             << "min_exp," << "max_exp," << "integers," << "total_runs," << "total_runtime" << "\n";
    }

    void write_to_file(std::fstream& file) {
        file << function_name << ",";
        file << prng_name << ",";
        file << num_buckets << ",";
        file << buffer_size << ",";
        file << threshold << ",";
        file << min_exp << ",";
        file << max_exp << ",";
        file << size << ",";
        file << total_runs << ",";
        file << total_runtime.count() << "\n";
    }
};

void benchmark_inplace_scatter_shuffle_pcg() {
    benchmark_param benchmark;
    benchmark.function_name = "inplace_scatter_shuffle";
    benchmark.prng_name = "pcg64";
    benchmark.num_buckets = NUM_BUCKETS;
    benchmark.buffer_size = BUFFER_SIZE;
    benchmark.threshold = THRESHOLD;
    benchmark.min_exp = 19; // 0
    benchmark.max_exp = 19; // 29
    benchmark.min_exp = 0;
    benchmark.max_exp = 33; // 29 for my mac, 30 for my windows machine, 33 for the uni-machine
    benchmark.size = 0;
    benchmark.total_runs = 0;
    benchmark.total_runtime = std::chrono::nanoseconds::zero();
    benchmark.DEFAULT_RUNS = 10;
    benchmark.MIN_DURATION = std::chrono::milliseconds(100);

    pcg_extras::seed_seq_from<std::random_device> seed_source;
    pcg64 generator(seed_source);

    std::filesystem::path path = create_csv_path(benchmark.num_buckets, benchmark.buffer_size, benchmark.threshold);

    std::fstream my_file;
    my_file.open(path, std::ios::out);
    if (my_file.is_open()) {
        std::cout << "Starting benchmark with " << benchmark.num_buckets << " buckets...\n";

        // Creating CSV headers
        benchmark.create_header(my_file);

        // Initiliazing vector with maximum size
        std::vector<std::size_t> vec(std::pow(2, benchmark.max_exp));
        std::iota(vec.begin(), vec.end(), 0);
        std::span vector_span {vec};

        for (std::size_t i = benchmark.min_exp; i <= benchmark.max_exp; i++) {
            benchmark.size = std::pow(2, i);
            std::cout << std::setw(static_cast<size_t>(std::log10(benchmark.max_exp))) << i + 1 << "/" << benchmark.max_exp + 1 << " ";
            std::cout << "Setting size = " << std::setw(static_cast<size_t>(std::log10(std::pow(2, benchmark.max_exp)))) << benchmark.size;
            std::cout << " " << "which needs " << sizeof(size_t) * benchmark.size << " Bytes of storage.\n";

            // Getting the first size elements
            std::span view = vector_span.first(benchmark.size);

            benchmark.total_runs = benchmark.DEFAULT_RUNS;
            while (true) {
                auto start = std::chrono::steady_clock::now();
                for (std::size_t i = 0; i < benchmark.total_runs; i++) {
                    inplace_scatter_shuffle(view, generator);
                }
                auto end = std::chrono::steady_clock::now();

                benchmark.total_runtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                if (benchmark.total_runtime >= static_cast<std::chrono::nanoseconds>(benchmark.MIN_DURATION)) {
                    benchmark.write_to_file(my_file);
                    std::cout << "Total runtime: " << std::setw(18) << benchmark.total_runtime.count() << " ns" << "\n";
                    break;
                }
                benchmark.total_runs *= 10;
            }
            std::cout << "\n";
        }

        std::cout << "Benchmark done!" << std::endl;

        my_file.close();
    } else {
        std::cout << "ERROR: File not found!" << "\n";
    }
} 

void benchmark_fy_shuffle_32() {
    benchmark_param benchmark;
    benchmark.function_name = "fy_shuffle_32";
    benchmark.prng_name = "pcg64";
    benchmark.num_buckets = NUM_BUCKETS;
    benchmark.buffer_size = BUFFER_SIZE;
    benchmark.threshold = THRESHOLD;
    benchmark.min_exp = 0;
    benchmark.max_exp = 33; // 29 for my mac, 30 for my windows machine, 33 for the uni-machine
    benchmark.size = 0;
    benchmark.total_runs = 0;
    benchmark.total_runtime = std::chrono::nanoseconds::zero();
    benchmark.DEFAULT_RUNS = 5;
    benchmark.MIN_DURATION = std::chrono::milliseconds(100);

    pcg_extras::seed_seq_from<std::random_device> seed_source;
    pcg64 generator(seed_source);

    std::filesystem::path path = create_csv_path(benchmark.num_buckets, benchmark.buffer_size, benchmark.threshold);

    std::fstream my_file;
    my_file.open(path, std::ios::out);
    if (my_file.is_open()) {
        std::cout << "Starting benchmark with " << benchmark.num_buckets << " buckets...\n";

        // Creating CSV headers
        benchmark.create_header(my_file);

        // Initiliazing vector with maximum size
        std::vector<std::size_t> vec(std::pow(2, benchmark.max_exp));
        std::iota(vec.begin(), vec.end(), 0);
        std::span vector_span {vec};

        for (std::size_t i = benchmark.min_exp; i <= benchmark.max_exp; i++) {
            benchmark.size = std::pow(2, i);
            std::cout << std::setw(static_cast<size_t>(std::log10(benchmark.max_exp))) << i + 1 << "/" << benchmark.max_exp + 1 << " ";
            std::cout << "Setting size = " << std::setw(static_cast<size_t>(std::log10(std::pow(2, benchmark.max_exp)))) << benchmark.size;
            std::cout << " " << "which needs " << sizeof(size_t) * benchmark.size << " Bytes of storage.\n";

            // Getting the first size elements
            std::span view = vector_span.first(benchmark.size);

            benchmark.total_runs = benchmark.DEFAULT_RUNS;
            while (true) {
                auto start = std::chrono::steady_clock::now();
                for (std::size_t i = 0; i < benchmark.total_runs; i++) {
                    inplace_scatter_shuffle(view, generator);
                }
                auto end = std::chrono::steady_clock::now();

                benchmark.total_runtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                if (benchmark.total_runtime >= static_cast<std::chrono::nanoseconds>(benchmark.MIN_DURATION)) {
                    benchmark.write_to_file(my_file);
                    std::cout << "Total runtime: " << std::setw(18) << benchmark.total_runtime.count() << " ns" << "\n";
                    break;
                }
                benchmark.total_runs *= 10;
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
