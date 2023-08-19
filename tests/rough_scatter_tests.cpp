#include <gtest/gtest.h>
#include <boost/math/distributions/binomial.hpp>
    using boost::math::binomial;

#include <cip_shuffle.hpp>

// Helper function to calculate the p-value.
double calc_p_value(std::size_t result, std::size_t sample_size, double prop) {
    // Boost documentation advises to use try/catch blocks.
    try {
        // Creating the binomial distribution
        binomial binom(sample_size, prop);
        // One-sided left-tail test-statistic distribution
        double left_tail = cdf(binom, result);
        // One-sided right-tail test-statistic distribution. Note that complement of cdf 
        // means more than k successes.
        // double right_tail = cdf(complement(binom, result)) + pdf(binom, result);
        double right_tail = (result == 0) ? (1) : (1 - cdf(binom, result - 1));
        return 2 * std::min(left_tail, right_tail);
    } catch(const std::exception& e) {
        std::cout << "\n""Message from thrown exception was:\n " << e.what() << "\n";
        throw;
    }
}

template<typename T, std::size_t K>
void create_buckets(std::span<T> span , std::array<bucket_limits, K> &buckets) {
    for (std::size_t i = 0; i < K; i++) {
        buckets[i].begin = static_cast<std::size_t>(span.size()) * i / K;
        buckets[i].staged = static_cast<std::size_t>(span.size()) * i / K;;
        buckets[i].end = static_cast<std::size_t>(span.size()) * (i+1) / K;
    }
}

//-------------------------------------------------------------------------------------------------
// Code to be tested
//-------------------------------------------------------------------------------------------------

class RoughScatterTest : public testing::Test {
    protected:
        int seed;
        size_t size;
        double confidence;

        void SetUp() override {
            seed = 12345;
            size = 1024;
            confidence = 0.05;
        }
};

TEST_F(RoughScatterTest, OnlyOneBucketFull_4) {
    constexpr size_t num_buckets = 4;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;

    // Generatinf a vector of size size
    std::vector<std::size_t> V(size);
    std::iota(V.begin(), V.end(), 0);
    std::span vector_span {V};

    create_buckets(vector_span, buckets);

    rough_scatter<num_buckets>(vector_span, buckets, generator);

    int result = 0;
    for (size_t i = 0; i < num_buckets; i++) {
        if (buckets[i].staged == buckets[i].end) {
            result++;
        }
    }
    
    EXPECT_EQ(1, result);
}

TEST_F(RoughScatterTest, OnlyOneBucketFull_64) {
    constexpr size_t num_buckets = 64;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;

    // Generatinf a vector of size size
    std::vector<std::size_t> V(size);
    std::iota(V.begin(), V.end(), 0);
    std::span vector_span {V};

    create_buckets(vector_span, buckets);

    rough_scatter<num_buckets>(vector_span, buckets, generator);

    int result = 0;
    for (size_t i = 0; i < num_buckets; i++) {
        if (buckets[i].staged == buckets[i].end) {
            result++;
        }
    }
    
    EXPECT_EQ(1, result);
}

TEST_F(RoughScatterTest, OnlyOneBucketFull_256) {
    constexpr size_t num_buckets = 256;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;

    // Generatinf a vector of size size
    std::vector<std::size_t> V(size);
    std::iota(V.begin(), V.end(), 0);
    std::span vector_span {V};

    create_buckets(vector_span, buckets);

    rough_scatter<num_buckets>(vector_span, buckets, generator);

    int result = 0;
    for (size_t i = 0; i < num_buckets; i++) {
        if (buckets[i].staged == buckets[i].end) {
            result++;
        }
    }
    
    EXPECT_EQ(1, result);
}

TEST_F(RoughScatterTest, BucketLimits_4) {
    constexpr size_t num_buckets = 4;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;

    // Generatinf a vector of size size
    std::vector<std::size_t> V(size);
    std::iota(V.begin(), V.end(), 0);
    std::span vector_span {V};

    create_buckets(vector_span, buckets);

    rough_scatter<num_buckets>(vector_span, buckets, generator);

    int result = 0;
    for (size_t i = 0; i < num_buckets; i++) {
        bucket_limits bucket = buckets[i];
        result += bucket.num_placed() + bucket.num_staged();
    }
    
    EXPECT_EQ(size, result);
}

TEST_F(RoughScatterTest, BucketLimits_64) {
    constexpr size_t num_buckets = 64;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;

    // Generatinf a vector of size size
    std::vector<std::size_t> V(size);
    std::iota(V.begin(), V.end(), 0);
    std::span vector_span {V};

    create_buckets(vector_span, buckets);

    rough_scatter<num_buckets>(vector_span, buckets, generator);

    int result = 0;
    for (size_t i = 0; i < num_buckets; i++) {
        bucket_limits bucket = buckets[i];
        result += bucket.num_placed() + bucket.num_staged();
    }
    
    EXPECT_EQ(size, result);
}

TEST_F(RoughScatterTest, BucketLimits_256) {
    constexpr size_t num_buckets = 256;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;

    // Generatinf a vector of size size
    std::vector<std::size_t> V(size);
    std::iota(V.begin(), V.end(), 0);
    std::span vector_span {V};

    create_buckets(vector_span, buckets);

    rough_scatter<num_buckets>(vector_span, buckets, generator);

    int result = 0;
    for (size_t i = 0; i < num_buckets; i++) {
        bucket_limits bucket = buckets[i];
        result += bucket.num_placed() + bucket.num_staged();
    }
    
    EXPECT_EQ(size, result);
}

TEST_F(RoughScatterTest, BucketFirstFinished_4) {
    constexpr size_t num_buckets = 4;
    size_t sample_size = 10 * num_buckets * num_buckets;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;
    std::vector<size_t> result(num_buckets);

    for (size_t l = 0; l < sample_size; l++) {
        // Generatinf a vector of size size
        std::vector<std::size_t> V(size);
        std::iota(V.begin(), V.end(), 0);
        std::span vector_span {V};

        create_buckets(vector_span, buckets);

        rough_scatter<num_buckets>(vector_span, buckets, generator);

        for (size_t i = 0; i < num_buckets; i++) {
            if (buckets[i].staged == buckets[i].end) {
                result[i]++;
                break;
            }
        }
    }

    for (size_t i = 0; i < num_buckets; i++) {
        double p_value = calc_p_value(result[i], sample_size, 1.0 / static_cast<double>(num_buckets));
        bool reject = false;
        if (p_value < confidence / static_cast<double>(num_buckets)) {
            reject = true;
        }
        EXPECT_EQ(false, reject) << i << " ";
    }

    for (auto &num : result) {
        std::cout << num << " ";
    }
}

TEST_F(RoughScatterTest, BucketFirstFinished_64) {
    constexpr size_t num_buckets = 64;
    size_t sample_size = 10 * num_buckets * num_buckets;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;
    std::vector<size_t> result(num_buckets);

    for (size_t l = 0; l < sample_size; l++) {
        // Generatinf a vector of size size
        std::vector<std::size_t> V(size);
        std::iota(V.begin(), V.end(), 0);
        std::span vector_span {V};

        create_buckets(vector_span, buckets);

        rough_scatter<num_buckets>(vector_span, buckets, generator);

        for (size_t i = 0; i < num_buckets; i++) {
            if (buckets[i].staged == buckets[i].end) {
                result[i]++;
                break;
            }
        }
    }

    for (size_t i = 0; i < num_buckets; i++) {
        double p_value = calc_p_value(result[i], sample_size, 1.0 / static_cast<double>(num_buckets));
        bool reject = false;
        if (p_value < confidence / static_cast<double>(num_buckets)) {
            reject = true;
        }
        EXPECT_EQ(false, reject) << i << " ";
    }

    for (auto &num : result) {
        std::cout << num << " ";
    }
}

TEST_F(RoughScatterTest, BucketFirstFinished_256) {
    constexpr size_t num_buckets = 256;
    size_t sample_size = 10 * num_buckets * num_buckets;

    std::mt19937_64 generator(seed);

    std::array<bucket_limits, num_buckets> buckets;
    std::vector<size_t> result(num_buckets);

    for (size_t l = 0; l < sample_size; l++) {
        // Generatinf a vector of size size
        std::vector<std::size_t> V(size);
        std::iota(V.begin(), V.end(), 0);
        std::span vector_span {V};

        create_buckets(vector_span, buckets);

        rough_scatter<num_buckets>(vector_span, buckets, generator);

        for (size_t i = 0; i < num_buckets; i++) {
            if (buckets[i].staged == buckets[i].end) {
                result[i]++;
                break;
            }
        }
    }

    for (size_t i = 0; i < num_buckets; i++) {
        double p_value = calc_p_value(result[i], sample_size, 1.0 / static_cast<double>(num_buckets));
        bool reject = false;
        if (p_value < confidence / static_cast<double>(num_buckets)) {
            reject = true;
        }
        EXPECT_EQ(false, reject) << i << " ";
    }

    for (auto &num : result) {
        std::cout << num << " ";
    }
}