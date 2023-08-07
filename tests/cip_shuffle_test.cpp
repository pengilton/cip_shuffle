#include <gtest/gtest.h>
#include <boost/math/distributions/binomial.hpp>
    using boost::math::binomial;

#include <cip_shuffle.hpp>


double calc_p_value(std::size_t result, std::size_t sample_size, double prop) {
    // Boost documentation advises to use try/catch blocks.
    try {
        // Creating the binomial distribution
        binomial binom(sample_size, prop);
        // One-sided left-tail test-statistic distribution
        double left_tail = cdf(binom, result);
        // One-sided right-tail test-statistic distribution. Note that complement of cdf 
        // means more than k successes.
        double right_tail = cdf(complement(binom, result)) + pdf(binom, result);
        return 2 * std::min(left_tail, right_tail);
    } catch(const std::exception& e) {
        std::cout << "\n""Message from thrown exception was:\n " << e.what() << "\n";
        throw;
    }
}


TEST(cip_shuffle_test, p_test) {
    int seed = 0;
    std::default_random_engine generator(seed);

    double confidence = 0.05;

    constexpr std::size_t num_buckets = 8;
    constexpr std::size_t size = 50;

    std::size_t sample_size = size * size * size;
    std::array<std::array<std::size_t, size>, size> results {};

    for (std::size_t l = 0; l < sample_size; l++) {
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
        }
    }
}
