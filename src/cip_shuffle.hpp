#ifndef CIP_SHUFFLE_HPP
#define CIP_SHUFFLE_HPP

#include <random>
#include <array>
#include <vector>
#include <span>
#include <algorithm>
#include <numeric>
#include <cmath>

#ifdef LOG_NUM_BUCKETS_VAR
    constexpr std::size_t LOG_NUM_BUCKETS = LOG_NUM_BUCKETS_VAR;
#else
    constexpr std::size_t LOG_NUM_BUCKETS = 5;  // Default is 7; 2, 5, 7
#endif

#ifdef LOG_BUFFER_SIZE_VAR
    constexpr std::size_t LOG_BUFFER_SIZE = LOG_BUFFER_SIZE_VAR;
#else
    constexpr std::size_t LOG_BUFFER_SIZE = 8;  // Default is 8, Might change
#endif

#ifdef LOG_THRESHOLD_VAR
    constexpr std::size_t LOG_THRESHOLD = LOG_THRESHOLD_VAR;                   
#else
    constexpr std::size_t LOG_THRESHOLD = 20;   // Default is 18; 8, 12, 18
#endif

#ifdef LOG_BUFFER_THRESHOLD_VAR
    constexpr std::size_t LOG_BUFFER_THRESHOLD = LOG_BUFFER_THRESHOLD_VAR;                   
#else
    constexpr std::size_t LOG_BUFFER_THRESHOLD = 20;   // Default is 18; 8, 12, 18
#endif

constexpr std::size_t NUM_BUCKETS = 1 << LOG_NUM_BUCKETS;
constexpr std::size_t BUFFER_SIZE = 1 << LOG_BUFFER_SIZE; 
constexpr std::size_t THRESHOLD = 1 << LOG_THRESHOLD;
constexpr std::size_t BUFFER_THRESHOLD = 1 << LOG_BUFFER_THRESHOLD;

// Bucket as data structure
struct bucket_limits {
    std::size_t begin;
    std::size_t staged;
    std::size_t end;

    std::size_t num_total() { return end - begin; }
    std::size_t num_placed() { return staged - begin; } 
    std::size_t num_staged() { return end - staged; } 
};

// Note taht gen is a 64-bit generator.
// This function is from the following paper:
// Daniel Lemire. 2019. Fast Random Integer Generation in an Interval. ACM Trans. Model. Comput. Simul. 29, 1, Article 3 (January 2019), 12 pages. https://doi.org/10.1145/3230636
template<typename RNG>
std::uint32_t my_uniform_int_distribution_32(std::uint32_t s, RNG &gen) {
    // This should generate a 64 Bit word using any generator. We 
    // save it as a 32 Bit word. s should be random from [0, 2^32).
    std::uint32_t x = static_cast<uint32_t>(gen());
    std::uint64_t m = static_cast<std::uint64_t>(x) * static_cast<std::uint64_t>(s);
    std::uint32_t l = std::uint32_t(m);
    if (l < s) {
        std::uint32_t t = -s % s;
        while (l < t) {
            x = static_cast<uint32_t>(gen());
            std::uint64_t m = static_cast<std::uint64_t>(x) * static_cast<std::uint64_t>(s);
            l = std::uint32_t(m);
        }
    }
    return m >> 32;
}

// Daniel Lemire's fast random integer in an interval [0, s) algorithm. This should be part 
// of the std::uniform_int_distribution already. 
// This function is from the following paper:
// Daniel Lemire. 2019. Fast Random Integer Generation in an Interval. ACM Trans. Model. Comput. Simul. 29, 1, Article 3 (January 2019), 12 pages. https://doi.org/10.1145/3230636
template<typename RNG>
std::uint64_t my_uniform_int_distribution_64(std::uint64_t s, RNG &gen) {
    // This should generate a 64 Bit word using any generator. We 
    // save it as a 64 Bit word. s should be random from [0, 2^64).
    std::uint64_t x = gen();
    __uint128_t m = static_cast<__uint128_t>(x) * static_cast<__uint128_t>(s);
    std::uint64_t l = std::uint64_t(m);
    if (l < s) {
        std::uint64_t t = -s % s;
        while (l < t) {
            x = gen();
            __uint128_t m = static_cast<__uint128_t>(x) * static_cast<__uint128_t>(s);
            l = std::uint64_t(m);
        }
    }
    return m >> 64;
}

// A simple Fisher-Yates implementation
template<typename T, typename RNG>
void fisher_yates_shuffle(std::span<T> data_span, RNG &gen) {
    if (!data_span.empty()) {
        for (std::size_t i = data_span.size() - 1; i > 0; i--) {
            // uniform sample from [0, i]
            std::uniform_int_distribution<> distrib(0UL, i);
            std::size_t j = distrib(gen);

            using std::swap;
            swap(data_span[i], data_span[j]);
        }
    }
}

// A simple Fisher-Yates implementation with our own 32-bit uniform_int_distribution
template<typename T, typename RNG>
void fisher_yates_shuffle_32(std::span<T> data_span, RNG &gen) {
    if (!data_span.empty()) {
        for (std::size_t i = data_span.size() - 1; i > 0; i--) {
            // uniform sample from [0, i+1)
            std::size_t j = my_uniform_int_distribution_32(i + 1, gen);

            using std::swap;
            swap(data_span[i], data_span[j]);
        }
    }
}

// A simple Fisher-Yates implementation with our own 64-bit uniform_int_distribution
template<typename T, typename RNG>
void fisher_yates_shuffle_64(std::span<T> data_span, RNG &gen) {
    if (!data_span.empty()) {
        for (std::size_t i = data_span.size() - 1; i > 0; i--) {
            // uniform sample from [0, i+1)
            std::size_t j = my_uniform_int_distribution_64(i + 1, gen);

            using std::swap;
            swap(data_span[i], data_span[j]);
        }
    }
}

// Buffered version of Fisher-Yates as in Daniel Lemire's paper.
// Daniel Lemire. 2019. Fast Random Integer Generation in an Interval. ACM Trans. Model. Comput. Simul. 29, 1, Article 3 (January 2019), 12 pages. https://doi.org/10.1145/3230636
template<typename T, typename RNG>
void buffered_fisher_yates_shuffle_32(std::span<T> data_span, RNG &gen) {
    std::size_t i = data_span.size() - 1;
    std::array<std::size_t, BUFFER_SIZE> buffer{};

    for (; i >= BUFFER_SIZE; i -= BUFFER_SIZE) {
        for (std::size_t k = 0; k < BUFFER_SIZE; k++) {
            std::size_t j = my_uniform_int_distribution_32(static_cast<uint32_t>(i - k + 1), gen);
            buffer[k] = j;
        }
        for (std::size_t k = 0; k < BUFFER_SIZE; k++) {
            using std::swap;
            swap(data_span[i - k], data_span[buffer[k]]);
        }
    }
    while (i > 0) {
        std::size_t j = my_uniform_int_distribution_32(static_cast<uint32_t>(i + 1), gen);
        using std::swap;
        swap(data_span[i], data_span[j]);
        i--;
    }
}

// Buffered version of Fisher-Yates as in Daniel Lemire's paper.
// Daniel Lemire. 2019. Fast Random Integer Generation in an Interval. ACM Trans. Model. Comput. Simul. 29, 1, Article 3 (January 2019), 12 pages. https://doi.org/10.1145/3230636
template<typename T, typename RNG>
void buffered_fisher_yates_shuffle_64(std::span<T> data_span, RNG &gen) {
    std::size_t i = data_span.size() - 1;
    std::array<std::size_t, BUFFER_SIZE> buffer{};

    for (; i >= BUFFER_SIZE; i -= BUFFER_SIZE) {
        for (std::size_t k = 0; k < BUFFER_SIZE; k++) {
            std::size_t j = my_uniform_int_distribution_64(i - k + 1, gen);
            buffer[k] = j;
        }
        for (std::size_t k = 0; k < BUFFER_SIZE; k++) {
            using std::swap;
            swap(data_span[i - k], data_span[buffer[k]]);
        }
    }
    while (i > 0) {
        std::size_t j = my_uniform_int_distribution_64(i + 1, gen);
        using std::swap;
        swap(data_span[i], data_span[j]);
        i--;
    }
}


// Depending on data_span we either use the faster fisher_yates_shuffle_32 fucntion or switch to one of the buffered variants.
template<typename T, typename RNG>
void buffered_fisher_yates_shuffle(std::span<T> data_span, RNG &gen) {
    if (data_span.size() <= BUFFER_THRESHOLD) {
        fisher_yates_shuffle_32(data_span, gen);
    } else if (data_span.size() < std::numeric_limits<std::uint32_t>::max()) {
        buffered_fisher_yates_shuffle_32(data_span, gen);
    } else {
        buffered_fisher_yates_shuffle_64(data_span, gen);
    }
}

// A noncontinuous variant of the fisher-yates shuffle algorithm. The works basically like the 
// fisher-yates shuffle algorithm but we use the additional information given from the bucekts
// to calcualte some offset value to reach the noncontinuous staged sections in the data_span.
template<size_t K, typename T, typename RNG> 
void noncontinuous_fisher_yates_shuffle(std::span<T> data_span, std::array<bucket_limits, K> &buckets, RNG &gen) {
    if (data_span.empty()) {
        return; 
    }

    // At index i we store the total number of staged items left of bucket i, 
    // basically an exclusive prefix sum of the staged items.
    std::array<size_t, K> num_staged_items_left {};
    for (std::size_t i = 1; i < K; i++) {
        num_staged_items_left[i] = num_staged_items_left[i - 1] + buckets[i - 1].num_staged();
    }

    // Total amount of staged items
    std::size_t stash_size = num_staged_items_left[K - 1] + buckets[K - 1].num_staged();

    for (std::size_t index = stash_size - 1; index > 0; index--) {
        std::size_t random_index = my_uniform_int_distribution_64(index + 1, gen);

        // We search for the bucket which contains the index-th staged item
        std::size_t bucket_num_index = 0;
        for (std::size_t b_num = 1; b_num < K; b_num++) {
            if (num_staged_items_left[b_num] <= index) {
                bucket_num_index = b_num;
            } else {
                break;
            }
        }

        // We search for the bucket which contains the random_index-th staged item
        std::size_t bucket_num_random_index = 0;
        for (std::size_t b_num = 1; b_num < K; b_num++) {
            if (num_staged_items_left[b_num] <= random_index) {
                bucket_num_random_index = b_num;
            } else {
                break;
            }
        }
        
        // We map index to the actual location in the data_span
        std::size_t offset_i = index - num_staged_items_left[bucket_num_index];
        std::size_t i = buckets[bucket_num_index].staged + offset_i;

        // We map random_index to the actual location in the data_span
        std::size_t offset_j = random_index - num_staged_items_left[bucket_num_random_index];
        std::size_t j = buckets[bucket_num_random_index].staged + offset_j;

        using std::swap;
        swap(data_span[i], data_span[j]);
    }
}


// A function which puts the staged items into one continuous segment.
// Ideally the staged items will fit into one bucket.
// This code is based on the implemenation in https://github.com/manpen/rip_shuffle?tab=readme-ov-file
template<size_t K, typename T, typename RNG> 
void shuffle_stashes(std::span<T> data_span, std::array<bucket_limits, K> &buckets, RNG &gen) {
    size_t stash_size = 0;
    for (auto& bucket : buckets) {
        stash_size += bucket.num_staged();
    }

    if (stash_size <= buckets[K - 1].num_total()) {
        compact_stashes(data_span, buckets, stash_size);
        buffered_fisher_yates_shuffle(data_span.last(stash_size), gen);
        compact_stashes(data_span, buckets, stash_size);
    } else {
        // We use a rather unefficient method to shuffle the remaining stashed items. 
        // This method is not intended to be highly optimised because it is very likely 
        // that we have to fall back to this. 
        noncontinuous_fisher_yates_shuffle(data_span, buckets, gen); 
    }
}

// Helper function. 
// This code is based on the implemenation in https://github.com/manpen/rip_shuffle?tab=readme-ov-file
template<size_t K, typename T> 
void compact_stashes(std::span<T> data_span, std::array<bucket_limits, K> &buckets, size_t stash_size) {
    size_t remaining_items = stash_size;
    for (size_t i = 0; i < K - 1; i++) {
        bucket_limits bucket = buckets[i];
        std::swap_ranges(data_span.begin() + bucket.staged, 
                         data_span.begin() + bucket.end, 
                         data_span.end() - remaining_items);
        remaining_items -= bucket.num_staged();
    }
}

// I could leave out the argument with n because I am to get LOG_NUM_BUCKETS without it
// DEPRECATED
template<typename T, size_t K, typename RNG>
void uniform_n_bit_numbners(std::size_t n, std::array<T, K>& buffer, RNG &gen) {
    std::uint64_t bitmask = (1UL << n) - 1;

    std::uint64_t x = gen();
    for (std::size_t i = 0; i < buffer.size(); i++) {
        std::uint64_t chunk = static_cast<std::uint64_t>(x & bitmask); // Extract the lowest n bits
        buffer[i] = chunk;
        x = x >> n;
    }
}

// Rough Scatter
template<std::size_t K, typename T, typename RNG>
void rough_scatter(std::span<T> data_span, std::array<bucket_limits, K> &buckets, RNG &gen) {
    // bitmask to get lower bits 
    constexpr std::uint64_t bitmask = (1UL << LOG_NUM_BUCKETS) - 1;

    // counter which shows how many bits are left
    std::size_t bits_left = 64;
    // bits will hold 
    std::uint64_t random_bits = gen();

    while (true) {
        // We generate a new random index from random_bits
        std::uint64_t j = static_cast<std::uint64_t>(random_bits & bitmask);
        random_bits = random_bits >> LOG_NUM_BUCKETS;
        bits_left -= LOG_NUM_BUCKETS;

        if (j != 0) {
            using std::swap;
            std::size_t s_0 = buckets[0].staged;
            std::size_t s_j = buckets[j].staged;
            swap(data_span[s_0], data_span[s_j]);
        }

        buckets[j].staged++;

        if (buckets[j].staged == buckets[j].end) {
            break;
        }

        // We check if we have to generate new random bits
        if (bits_left < LOG_NUM_BUCKETS) {
            random_bits = gen();
            bits_left = 64;
        }
    }
}

// Fine Scatter
template<std::size_t K, typename T, typename RNG>
void fine_scatter(std::span<T> data_span, std::array<bucket_limits, K> &buckets, RNG &gen) {
    std::array<size_t, K> num_of_placed_items {};
    std::array<size_t, K> num_to_be_placed_items {};
    std::array<size_t, K> final_bucket_sizes {};

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
        final_bucket_sizes[i] = num_of_placed_items[i] + num_to_be_placed_items[i];
    }

    // We sweep from left to right. Similar to Penschuck's code. We don't precalcute the C values.
    // Penschuck's code https://github.com/manpen/rip_shuffle?tab=readme-ov-file
    long long growth_needed_left = 0;
    for (std::size_t i = 0; i+1 < K; i++) {
        size_t reservation_for_left = std::max(growth_needed_left, static_cast<long long>(0));
        size_t target_with_reservation = final_bucket_sizes[i] + reservation_for_left;
        while (buckets[i].num_total() > target_with_reservation) {
            // Swapping the last staged item of bucket i with the last placed item of bucket i+1
            using std::swap;
            swap(data_span[buckets[i].end - 1], data_span[buckets[i+1].staged - 1]);
            // Adjusting the buckets
            buckets[i].end--;
            buckets[i+1].begin--;
            buckets[i+1].staged--;
        }
        growth_needed_left += final_bucket_sizes[i] - buckets[i].num_total();
    }
    
    // We sweep from right to left
    for (size_t i = K-1; i > 0; i--) {
        while (buckets[i].num_total() > final_bucket_sizes[i]) {
            // Swapping first placed item in bucket i with the first staged item in bucket i
            using std::swap;
            swap(data_span[buckets[i].begin], data_span[buckets[i].staged]);
            // Adjusting the buckets
            buckets[i].staged++;
            buckets[i].begin++;
            buckets[i-1].end++;
        }
    }

    // Now we shuffle the stashes
    shuffle_stashes(data_span, buckets, gen);
}

// Our InplaceScatterShuffle implementation
template<typename T, typename RNG>
void inplace_scatter_shuffle(std::span<T> data_span, RNG &gen) {
    // Might be unnecessary
    if (data_span.empty()) {
        return;
    }

    if (data_span.size() <= THRESHOLD) {
        buffered_fisher_yates_shuffle(data_span, gen);
        return;
    }

    // Maybe I should creade a small fucntion which creates the buckets to make this function 
    // a bit cleaner.
    std::array<bucket_limits, NUM_BUCKETS> buckets;
    for (std::size_t i = 0; i < NUM_BUCKETS; i++) {
        buckets[i].begin = static_cast<std::size_t>(data_span.size()) * i / NUM_BUCKETS;
        buckets[i].staged = static_cast<std::size_t>(data_span.size()) * i / NUM_BUCKETS;;
        buckets[i].end = static_cast<std::size_t>(data_span.size()) * (i+1) / NUM_BUCKETS;
    }

    // Rough Scatter
    rough_scatter(data_span, buckets, gen);

    // Fine Scatter which does the twosweap thing and assigns the last itmes 
    fine_scatter(data_span, buckets, gen);

    // Squentially calling inplace_scatter_schuffle on each bucket.
    // This part should be hhighly parallelisable.
    for (std::size_t i = 0; i < NUM_BUCKETS; i++) {
        // Might be unnecessary to create a span
        std::span bucket_span = data_span.subspan(buckets[i].begin, buckets[i].num_total());
        inplace_scatter_shuffle(bucket_span, gen);
    }
}

#endif /* CIP_SHUFFLE_HPP */