#include <iostream>
#include <random>
#include <array>
#include <vector>
#include <chrono>

// Fisher-Yates
// Does only work with integers right now
template<typename T>
void fisher_yates_shuffle(std::vector<T> &vec, std::default_random_engine &gen) {
    for (int i = 0; i < vec.size() - 1; i++) {
        // uniform sample from [i, N)
        std::uniform_int_distribution<> distrib(i, vec.size()-1);
        int j = distrib(gen);

        // swapping of values
        T temp = vec[i];
        vec[i] = vec[j];
        vec[j] = temp;
    }
}

// We will implenet the sequential version of scatter shuffle
template<typename T>
void scatter_shuffle(std::vector<T> &vec, int k, std::default_random_engine &gen) {
    if (vec.size() == 0) {
        return;
    }

    int small = 8;
    if (vec.size() <= small) {
        fisher_yates_shuffle(vec, gen);
        return;
    }

    // We have k buckets. A bucket is a vecotr for now. 
    std::vector<T> buckets[k];
    // Uniform distribution from 0 to k-1
    std::uniform_int_distribution<> distrib(0, k-1);

    for (int i = 0; i < vec.size(); i++) {
        // randomly choose a bucket
        int j = distrib(gen);
        // copies value to chosen bucket
        buckets[j].push_back(vec[i]);
    }

    int s = 0;
    for (int j = 0; j < k; j++) {
        scatter_shuffle(buckets[j], k, gen);
        std::copy(buckets[j].begin(), buckets[j].end(), vec.begin() + s);
        s += buckets[j].size();
    }
}

// Function to print stuff
template<typename T>
void print_vec(std::vector<T> &vec) {
    int numbers_per_line = 16;
    for (int i = 0; i < vec.size(); i++) {
        if ((i % numbers_per_line == 0) && (i != 0)) {
            std::cout << std::endl;
        }
        std::cout << vec[i] << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, char const *argv[]) {
    // Rabndom generator
    std::random_device rd;
    // use fixed seed to test
    // int seed = 0;
    int seed = rd();
    std::default_random_engine generator(seed);

    // vector size
    int size = 10000000;
    // amound of buckets
    int k = 16;
    // runs
    int runs = 50;

    for (int i = 0; i < runs; i++) {
        std::vector<int> V(size);
        std::iota(V.begin(), V.end(), 0);

        auto start = std::chrono::steady_clock::now();
        scatter_shuffle(V, k, generator);
        auto end = std::chrono::steady_clock::now();

        // print array after shuffle
        // print_vec(V);

        std::chrono::duration<double> elapsed_time = end - start;
        std::cout << "Runtime: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count() 
                  << "ms" << std::endl;
    }

    return 0;
}
