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
    constexpr std::size_t LOG_NUM_BUCKETS = 7;  // Default is 7; 2, 5, 7
#endif

#ifdef LOG_THRESHOLD_VAR
    constexpr std::size_t LOG_THRESHOLD = LOG_THRESHOLD_VAR;                   
#else
    constexpr std::size_t LOG_THRESHOLD = 18;   // Default is 18; 8, 12, 18
#endif

#ifdef LOG_BUFFER_SIZE_VAR
    constexpr std::size_t LOG_BUFFER_SIZE = LOG_BUFFER_SIZE_VAR;
#else
    constexpr std::size_t LOG_BUFFER_SIZE = 8;  // Default is 8, Might change
#endif

constexpr std::size_t NUM_BUCKETS = 1 << LOG_NUM_BUCKETS;
constexpr std::size_t THRESHOLD = 1 << LOG_THRESHOLD;
constexpr std::size_t BUFFER_SIZE = 1 << LOG_BUFFER_SIZE;                 

// Bucket as data structure
struct bucket_limits {
    std::size_t begin;
    std::size_t staged;
    std::size_t end;

    std::size_t num_total() { return end - begin; }
    std::size_t num_placed() { return staged - begin; } 
    std::size_t num_staged() { return end - staged; } 
};

// Note that gen is a 64 bit generator
template<typename RNG>
std::uint32_t my_uniform_int_distribution_32(std::uint32_t s, RNG &gen) {
    // This should generate a 64 Bit word using any generator. We 
    // save it as a 32 Bit word. x should be random from [0, 2^32).
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
// TODO; Have to make it safe to use eg. check if gen will return a 32 or 64 bit word.
template<typename RNG>
std::uint64_t my_uniform_int_distribution_64(std::uint64_t s, RNG &gen) {
    // This should generate a 64 Bit word using any generator. We 
    // save it as a 64 Bit word. x should be random from [0, 2^64).
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

// Buffered version of Fisher-Yates as in Daniel Lemire's paper.
template<typename T, typename RNG>
void buffered_fisher_yates_shuffle_32(std::span<T> data_span, RNG &gen) {
    // constexpr std::size_t BUFFER_SIZE = 1 << 8;

    std::size_t i = data_span.size() - 1;
    std::array<std::size_t, BUFFER_SIZE> buffer{};

    for (; i >= BUFFER_SIZE; i -= BUFFER_SIZE) {
        for (std::size_t k = 0; k < BUFFER_SIZE; k++) {
            // std::uniform_int_distribution<> distrib(0, i - k);
            // std::size_t j = distrib(gen);
            std::size_t j = my_uniform_int_distribution_32(static_cast<uint32_t>(i - k + 1), gen);
            buffer[k] = j;
        }
        for (std::size_t k = 0; k < BUFFER_SIZE; k++) {
            using std::swap;
            swap(data_span[i - k], data_span[buffer[k]]);
        }
    }
    while (i > 0) {
        // std::uniform_int_distribution<> distrib(0, i);
        // std::size_t j = distrib(gen);
        std::size_t j = my_uniform_int_distribution_32(static_cast<uint32_t>(i + 1), gen);
        using std::swap;
        swap(data_span[i], data_span[j]);
        i--;
    }
}

// Buffered version of Fisher-Yates as in Daniel Lemire's paper.
template<typename T, typename RNG>
void buffered_fisher_yates_shuffle_64(std::span<T> data_span, RNG &gen) {
    // constexpr std::size_t BUFFER_SIZE = 1 << 8;

    std::size_t i = data_span.size() - 1;
    std::array<std::size_t, BUFFER_SIZE> buffer{};

    for (; i >= BUFFER_SIZE; i -= BUFFER_SIZE) {
        for (std::size_t k = 0; k < BUFFER_SIZE; k++) {
            // std::uniform_int_distribution<> distrib(0, i - k);
            // std::size_t j = distrib(gen);
            std::size_t j = my_uniform_int_distribution_64(i - k + 1, gen);
            buffer[k] = j;
        }
        for (std::size_t k = 0; k < BUFFER_SIZE; k++) {
            using std::swap;
            swap(data_span[i - k], data_span[buffer[k]]);
        }
    }
    while (i > 0) {
        // std::uniform_int_distribution<> distrib(0, i);
        // std::size_t j = distrib(gen);
        std::size_t j = my_uniform_int_distribution_64(i + 1, gen);
        using std::swap;
        swap(data_span[i], data_span[j]);
        i--;
    }
}

// Buffered version of Fisher-Yates as in Daniel Lemire's paper.
template<typename T, typename RNG>
void buffered_fisher_yates_shuffle(std::span<T> data_span, RNG &gen) {
    // May change the numeric_limits to (1 << 32)
    if (data_span.size() <= std::numeric_limits<std::uint32_t>::max()) {
        buffered_fisher_yates_shuffle_32(data_span, gen);
    } else {
        buffered_fisher_yates_shuffle_64(data_span, gen);
    }
}

// A function which puts the staged items into one continuous segment.
// Ideally the staged items will fit into one bucket.
template<size_t K, typename T, typename RNG> 
void shuffle_stashes(std::span<T> data_span, std::array<bucket_limits, K> &buckets, RNG &gen) {
    size_t stash_size = 0;
    for (auto& bucket : buckets) {
        stash_size += bucket.num_staged();
    }

    if (stash_size <= buckets[K - 1].num_total()) {
        compact_stashes(data_span, buckets, stash_size);
        // fisher_yates_shuffle(data_span.last(stash_size), gen);
        // std::shuffle(data_span.end() - stash_size, data_span.end(), gen);
        // buffered_fisher_yates_shuffle(data_span.subspan(data_span.size() - stash_size, stash_size), gen);
        buffered_fisher_yates_shuffle(data_span.last(stash_size), gen);
        // buffered_fisher_yates_shuffle_64(data_span.last(stash_size), gen);
        compact_stashes(data_span, buckets, stash_size);
    } else {
        // Now we can assign the remaining staged items. We move the staged
        // items together. There migh be a better way here as well but we assume that
        // we won't have to fall back to this part of the code that often.
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

        // All staged items should be grouped together now. 
        // fisher_yates_shuffle(data_span.first(stack.size()), gen);
        // std::shuffle(data_span.begin(), data_span.begin() + stack.size(), gen);
        buffered_fisher_yates_shuffle(data_span.first(stack.size()), gen);
        // buffered_fisher_yates_shuffle(data_span.subspan(0, stash_size), gen);
        // buffered_fisher_yates_shuffle_64(data_span.first(stack.size()), gen);

        // Now reverting the reordering
        while (stack.size() > 0) {
            i--;
            using std::swap;
            swap(data_span[i], data_span[stack.back()]);
            stack.pop_back();
        }
    }
}

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
    // Uniform distribution from 0 to k-1
    // std::uniform_int_distribution<> distrib(0, K-1);
    // Could be named buffer as well, but I have a buffer in Fisher-Yates... 
    // Also this is hard-coded that we we devide a 64-bit number into multiple 
    // n-bit numbers because the generator will generate a 64-bit number. Though it 
    // is possible that someone will use a 32-bit generator. 
    constexpr std::size_t chunks_size = static_cast<std::size_t>(64 / LOG_NUM_BUCKETS);
    std::array<std::size_t, chunks_size> chunks {};

    uniform_n_bit_numbners(LOG_NUM_BUCKETS, chunks, gen);

    std::size_t i = 0;
    while (true) {
        // int std::size_t = distrib(gen);
        // std::size_t j = my_uniform_int_distribution_64(K, gen);
        std::size_t j = chunks[i];

        // Benchmark if this slows down the code
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

        i++;
        if (i >= chunks.size()) {
            uniform_n_bit_numbners(LOG_NUM_BUCKETS, chunks, gen);
            i = 0;
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
    
    /* long long growth_needed_left = 0;
    for (std::size_t i = 0; i+1 < K; i++) {
        size_t reservation_for_left = std::max(growth_needed_left, static_cast<long long>(0));
        size_t target_with_reservation = final_bucket_sizes[i] + reservation_for_left;
        if (buckets[i].num_total() > target_with_reservation) {
            size_t num_to_move = buckets[i].num_total() - target_with_reservation;
            // We check if there are enough placed items in bucket i+1.
            if (num_to_move > buckets[i+1].num_placed()) {
                std::swap_ranges(data_span.begin() + buckets[i].end - num_to_move, 
                                 data_span.begin() + buckets[i].end,
                                 data_span.begin() + buckets[i+1].begin);
            } else {
                std::swap_ranges(data_span.begin() + buckets[i].end - num_to_move, 
                                 data_span.begin() + buckets[i].end,
                                 data_span.begin() + buckets[i+1].staged - num_to_move);
            }
            buckets[i].end -= num_to_move;
            buckets[i+1].begin -= num_to_move;
            buckets[i+1].staged -= num_to_move;
        }
        growth_needed_left += final_bucket_sizes[i] - buckets[i].num_total();
    } */

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


// IpScShuf
// First draft of the inplace scatter shuffle algorithm based on chapter 3
template<typename T, typename RNG>
void inplace_scatter_shuffle(std::span<T> data_span, RNG &gen) {
    // Might be unnecessary
    if (data_span.empty()) {
        return;
    }

    if (data_span.size() <= THRESHOLD) {
        // fisher_yates_shuffle(data_span, gen);
        buffered_fisher_yates_shuffle(data_span, gen);
        // std::shuffle(data_span.begin(), data_span.end(), gen);
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