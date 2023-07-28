#include <iostream>
#include <random>
#include <array>
#include <vector>
#include <span>
#include <algorithm>
#include <numeric>
#include <cmath>
// #include <chrono>
#include <gtest/gtest.h>


// Bucket as data structure
struct bucket_limits {
    std::size_t begin;
    std::size_t staged;
    std::size_t end;

    std::size_t num_total() { return end - begin; }
    std::size_t num_placed() { return staged - begin; } 
    std::size_t num_staged() { return end - staged; } 
};


// Fisher-Yates
// Swapping of elements could be its own function. Maybe there is a way to swap elements
// in the standard library 
template<typename T>
void fisher_yates_shuffle(std::span<T> data_span, std::default_random_engine &gen) {
    if (!data_span.empty()) {
        for (std::size_t i = 0; i < data_span.size() - 1; i++) {
            // uniform sample from [i, N)
            std::uniform_int_distribution<> distrib(i, data_span.size()-1);
            std::size_t j = distrib(gen);

            // swapping of values -> might change this with std::swap
            using std::swap;
            swap(data_span[i], data_span[j]);
        }
    }
}


// Rough Scatter
// Might have to swap size_t and typename
template<std::size_t K, typename T>
void rough_scatter(std::span<T> data_span, std::array<bucket_limits, K> &buckets, std::default_random_engine &gen) {
    // Uniform distribution from 0 to k-1
    std::uniform_int_distribution<> distrib(0, K-1);

    // Lets remove bool = done and add if break
    while (true) {
        int j = distrib(gen);

        // Benchmark if this if slows down the code
        if (j != 0) {
            // They are helpful here
            using std::swap;
            std::size_t s_0 = buckets[0].staged;
            std::size_t s_j = buckets[j].staged;
            swap(data_span[s_0], data_span[s_j]);
        }

        buckets[j].staged++;

        if (buckets[j].staged == buckets[j].end) {
            break;
        }
    }
}

// Fine Scatter. I might have to implement that sweaping thing as a separate function
template<std::size_t K, typename T>
void fine_scatter(std::span<T> data_span, std::array<bucket_limits, K> &buckets, std::default_random_engine &gen) {
    std::array<size_t, K> num_of_placed_items {};
    std::array<size_t, K> num_to_be_placed_items {};
    std::array<size_t, K> total_bucket_sizes {};
    std::array<size_t, K> derivation_of_bucket_sizes {};
    std::array<size_t, K> needed_items_left_of_each_bucket {};

    std::size_t num_staged_items = 0;

    for (std::size_t i = 0; i < K; i++) {
        auto& bucket = buckets[i];
        num_of_placed_items[i] = bucket.num_placed();
        num_staged_items += bucket.num_staged();
    }
    
    // This line might be unnecessary 
    std::size_t ns = num_staged_items;
    for (std::size_t i = 0; i+1 < K; i++) {
        auto binom = std::binomial_distribution{ns, 1.0/static_cast<double>(K - i)};
        auto x = binom(gen);
        num_to_be_placed_items[i] = x;
        ns -= x;
    }
    num_to_be_placed_items[K-1] = ns;

    // Calculate the sum of both vectors and store it in n_f
    // Here I could use the std::transform function
    for (std::size_t i = 0; i < K; i++) {
        total_bucket_sizes[i] = num_of_placed_items[i] + num_to_be_placed_items[i];
    }

    for (std::size_t i = 0; i < K; i++) {
        // I use the actual bucket size instead of the mean
        derivation_of_bucket_sizes[i] = total_bucket_sizes[i] - buckets[i].num_total();
    }

    // Now we fill the vector needed_items_left_of_each_bucket
    std::exclusive_scan(derivation_of_bucket_sizes.begin(), derivation_of_bucket_sizes.end(), 
                        needed_items_left_of_each_bucket.begin(), 0, std::plus<>());

    // Now we can do the TwoSweep part 
    // We sweep from left to right
    for (std::size_t i = 0; i+1 < K; i++) {
        // TODO: Own function! See rust implementation
        // If bucket Bi is too large by more than ci we move excess staged items into
        // the staging area of bucket i+1
        while (buckets[i].num_total() > needed_items_left_of_each_bucket[i] + total_bucket_sizes[i]) {
            // Swapping the last staged item of bucket i with the last placed item of bucket i+1
            using std::swap;
            swap(data_span[buckets[i].end - 1], data_span[buckets[i+1].staged - 1]);
            // Adjusting the buckets
            buckets[i].end--;
            buckets[i+1].begin--;
            buckets[i+1].staged--;
        }
    }

    // We sweep from right to left
    for (size_t i = K-1; i > 0; i--) {
        while (buckets[i].num_total() > total_bucket_sizes[i]) {
            // Swapping first placed item in bucket i with the first staged item in bucket i
            using std::swap;
            swap(data_span[buckets[i].begin], data_span[buckets[i].staged]);
            // Adjusting the buckets
            buckets[i].staged++;
            buckets[i].begin++;
            buckets[i-1].end++;
        }
    }

    // TODO: Get rid of stack as it is not needed. 
    // Now we can assign the remaining staged items. We move the staged
    // items together.
    std::vector<std::size_t> stack;
    std::size_t i = 0;
    for (std::size_t j = 0; j < K; j++) {
        std::size_t l = buckets[j].staged;
        while (l < buckets[j].end) {
            using std::swap;
            swap(data_span[i], data_span[l]);
            stack.push_back(l);
            i++;
            l++;
        }
    }

    // All staged items should be grouped together now. Using Fisher-Yates here
    // Maybe use subspan here instad of using stack.size()
    fisher_yates_shuffle(data_span.first(stack.size()), gen);
    // std::shuffle(data_span.begin(), data_span.begin() + stack.size(), gen);

    // Now reverting the reordering
    while (stack.size() > 0) {
        i--;
        using std::swap;
        swap(data_span[i], data_span[stack.back()]);
        stack.pop_back();
    }
}


// IpScShuf
// First draft of the inplace scatter shuffle algorithm based on chapter 3
template<std::size_t K, typename T>
void inplace_scatter_shuffle(std::span<T> data_span, std::default_random_engine &gen) {
    // Might be unnecessary
    if (data_span.empty()) {
        return;
    }

    // Change to 256
    std::size_t small = 16;
    if (data_span.size() <= small) {
        fisher_yates_shuffle(data_span, gen);
        return;
    }

    // An vector which contains the triples of indices (bi, si, ei) but instead of having an 
    // array of triples we just put them next to each other. Hence each block of three elements in 
    // the array is one triple.
    // Maybe I should creade a small fucntion which creates the buckets to make this function 
    // a bit cleaner.
    std::array<bucket_limits, K> buckets;
    for (std::size_t i = 0; i < K; i++) {
        buckets[i].begin = static_cast<std::size_t>(data_span.size()) * i / K;
        buckets[i].staged = static_cast<std::size_t>(data_span.size()) * i / K;;
        buckets[i].end = static_cast<std::size_t>(data_span.size()) * (i+1) / K;
    }

    // Rough Scatter
    rough_scatter<K>(data_span, buckets, gen);

    // Fine Scatter which does the twosweap thing and assigns the last itmes 
    fine_scatter<K>(data_span, buckets, gen);

    // Squentially calling inplace_scatter_schuffle on each bucket.
    // This part should be hhighly parallelisable.
    for (std::size_t i = 0; i < K; i++) {
        // Might be unnecessary to create a span
        std::span bucket_span = data_span.subspan(buckets[i].begin, buckets[i].num_total());
        inplace_scatter_shuffle<K>(bucket_span, gen);
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

template<typename T, std::size_t N>
void my_print2(std::array<std::array<T, N>, N> &array) {
    for (std::size_t i = 0; i < N; i++) {
        for (std::size_t j = 0; j < N; j++) {
            std::cout << array[i][j] << " ";   
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

//-------------------------------------------------------------------------------------
// Helper functions fot the tests 
double nCk(double n, double k) {
    double result = 1.0;
    
    if (k > n / 2) {
        k = n - k;
    }

    for (std::size_t i = 1; i <= k; i++) {
        result *= (n + 1 - static_cast<double>(i));
        result /= i;
    }

    return result;
}

double my_binom_pdf(std::size_t k, std::size_t n, double p) {
    // double a = static_cast<double>(nCk(n, k));
    // double b = std::pow(p, k);
    // double c = std::pow(1-p, n-k);
    return static_cast<double>(nCk(n, k)) * std::pow(p, k) * std::pow(1-p, n-k);
}

double my_binom_cdf(std::size_t k, std::size_t n, double p) {
    double result = 0;

    for (std::size_t i = 0; i <= k; i++) {
        double x = my_binom_pdf(i, n, p);
        result += x;
    }

    return result;
}

double calc_p_value(std::size_t result, std::size_t sample_size, double prop) {
    double left_tail = my_binom_cdf(result, sample_size, prop);
    double right_tail = 1 - left_tail + my_binom_pdf(result, sample_size, prop);
    return 2 * std::min(left_tail, right_tail);
}


TEST(cip_shuffle_test, p_test) {
    std::random_device rd;
    int seed = 0;
    // seed = rd();
    std::default_random_engine generator(seed);

    double confidence = 0.05;

    constexpr std::size_t num_buckets = 8;
    constexpr std::size_t size = 50;

    std::size_t sample_size = size * size * size;
    std::array<std::array<std::size_t, size>, size> results {};

    for (std::size_t l = 0; l < sample_size; l++) {
        // std::array<std::size_t, test_array_size> array {};
        // std::iota(array.begin(), array.end(), 0);
        // std::span array_span {array};
        std::vector<std::size_t> V(size);
        std::iota(V.begin(), V.end(), 0);
        std::span vector_span {V};
        inplace_scatter_shuffle<num_buckets>(vector_span, generator);

        for (std::size_t j = 0; j < size; j++) {
            std::size_t i = vector_span[j];
            results[i][j]++;
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        for (std::size_t j = 0; j < size; j++) {
            double p_value = calc_p_value(results[i][j], sample_size, 1.0 / static_cast<double>(size));
            bool reject = false;
            if (p_value < confidence / static_cast<double>(std::pow(size, 2))) {
                reject = true;
            }
            EXPECT_EQ(false, reject);
            // ASSERT_EQ(true, reject) << "Error for number: " << i << " at pos: " << j;
        }
    }
}


/* int main() {
    std::random_device rd;
    int seed = 0;
    // seed = rd();
    std::default_random_engine generator(seed);

    double confidence = 0.05;

    constexpr std::size_t test_array_size = 10;
    constexpr std::size_t num_buckets = 4;

    std::size_t sample_size = test_array_size * test_array_size * test_array_size;

    std::array<std::array<std::size_t, test_array_size>, test_array_size> results {};

    for (std::size_t l = 0; l < sample_size; l++) {
        // std::array<std::size_t, test_array_size> array {};
        // std::iota(array.begin(), array.end(), 0);
        // std::span array_span {array};
        std::vector<std::size_t> V(test_array_size);
        std::iota(V.begin(), V.end(), 0);
        std::span vector_span {V};
        inplace_scatter_shuffle<num_buckets>(vector_span, generator);
        // fisher_yates_shuffle(vector_span, generator);

        for (std::size_t j = 0; j < test_array_size; j++) {
            std::size_t i = vector_span[j];
            results[i][j]++;
        }
    }

    for (std::size_t i = 0; i < test_array_size; i++) {
        for (std::size_t j = 0; j < test_array_size; j++) {
            double p_value = calc_p_value(results[i][j], sample_size, 1.0 / static_cast<double>(test_array_size));
            if (p_value < confidence / static_cast<double>(std::pow(test_array_size, 2))) {
                std::cout << "Error at: " << i << ", " << j << "\n";
            }
        }
    }

    my_print2(results);

    return 0;
} */