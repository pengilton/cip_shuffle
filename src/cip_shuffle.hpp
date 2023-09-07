#ifndef CIP_SHUFFLE_HPP
#define CIP_SHUFFLE_HPP

#include <random>
#include <array>
#include <vector>
#include <span>
#include <algorithm>
#include <numeric>
#include <cmath>


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


// Rough Scatter
// Might have to swap size_t and typename
template<std::size_t K, typename T, typename RNG>
void rough_scatter(std::span<T> data_span, std::array<bucket_limits, K> &buckets, RNG &gen) {
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
template<std::size_t K, typename T, typename RNG>
void fine_scatter(std::span<T> data_span, std::array<bucket_limits, K> &buckets, RNG &gen) {
    std::array<size_t, K> num_of_placed_items {};
    std::array<size_t, K> num_to_be_placed_items {};
    std::array<size_t, K> final_bucket_sizes {};
    // We opt for long long as we need negative numbers here. There is still an issue that size_t 
    // may have larger values than long long hence overflow. We ignore this for now.
    // std::array<long long, K> derivation_of_bucket_sizes {};
    // std::array<long long, K> needed_items_left_of_each_bucket {};

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

    /*
    // Might not be needed!!!
    for (std::size_t i = 0; i < K; i++) {
        // I use the actual bucket size instead of the mean
        derivation_of_bucket_sizes[i] = final_bucket_sizes[i] - buckets[i].num_total();
    }

    // Might not be needed!!!
    // Now we fill the vector needed_items_left_of_each_bucket
    std::exclusive_scan(derivation_of_bucket_sizes.begin(), derivation_of_bucket_sizes.end(), 
                        needed_items_left_of_each_bucket.begin(), 0, std::plus<>());
    
    // Now we can do the TwoSweep part 
    // We sweep from left to right
    for (std::size_t i = 0; i+1 < K; i++) {
        // TODO: Own function! See rust implementation
        // If bucket Bi is too large by more than ci we move excess staged items into
        // the staging area of bucket i+1
        while (buckets[i].num_total() > needed_items_left_of_each_bucket[i] + final_bucket_sizes[i]) {
            // Swapping the last staged item of bucket i with the last placed item of bucket i+1
            using std::swap;
            swap(data_span[buckets[i].end - 1], data_span[buckets[i+1].staged - 1]);
            // Adjusting the buckets
            buckets[i].end--;
            buckets[i+1].begin--;
            buckets[i+1].staged--;
        }
    } */

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
    // fisher_yates_shuffle(data_span.first(stack.size()), gen);
    std::shuffle(data_span.begin(), data_span.begin() + stack.size(), gen);

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
template<std::size_t K, typename T, typename RNG>
void inplace_scatter_shuffle(std::span<T> data_span, RNG &gen) {
    // Might be unnecessary
    if (data_span.empty()) {
        return;
    }

    // Change to 256
    constexpr std::size_t THRESHOLD = 256;
    if (data_span.size() < THRESHOLD) {
        // fisher_yates_shuffle(data_span, gen);
        std::shuffle(data_span.begin(), data_span.end(), gen);
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

#endif /* CIP_SHUFFLE_HPP */