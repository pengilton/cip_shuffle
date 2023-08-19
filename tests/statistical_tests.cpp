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

void my_print(std::vector<std::vector<std::size_t>> &matrix) {
    for (std::size_t i = 0; i < matrix.size(); i++) {
        for (std::size_t j = 0; j < matrix[i].size(); j++) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "\n";
    }
}

//-------------------------------------------------------------------------------------------------

class CipShuffleTestFixture : public testing::TestWithParam<std::size_t> {
    protected:
        int seed;
        double confidence;

        void SetUp() override {
            seed = 12345;
            confidence = 0.05;
        }
};

TEST_P(CipShuffleTestFixture, IndependenceTest) {
    constexpr std::size_t num_buckets = 4;

    std::mt19937_64 generator(seed);

    std::size_t param = GetParam();
    const std::size_t size = param;

    std::size_t sample_size = 10 * size * size;
    std::vector<std::vector<std::size_t>> results(size, std::vector<std::size_t>(size));

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

    // my_print(results);

    for (std::size_t i = 0; i < size; i++) {
        for (std::size_t j = 0; j < size; j++) {
            double p_value = calc_p_value(results[i][j], sample_size, 1.0 / static_cast<double>(size));
            bool reject = false;
            if (p_value < confidence / static_cast<double>(std::pow(size, 2))) {
                reject = true;
            }
            EXPECT_EQ(false, reject) << i << " " << j << " " << results[i][j];
        }
    }
}

TEST_P(CipShuffleTestFixture, IndependencePairsTest) {
    constexpr std::size_t num_buckets = 4;

    std::mt19937_64 generator(seed);

    std::size_t param = GetParam();
    const std::size_t size = param;
    const std::size_t m_size = size * size;

    std::size_t sample_size = 10 * m_size * m_size;
    std::vector<std::vector<std::size_t>> results(m_size, std::vector<std::size_t>(m_size));

    for (std::size_t l = 0; l < sample_size; l++) {
        std::vector<std::size_t> V(size);
        std::iota(V.begin(), V.end(), 0);
        std::span vector_span {V};
        inplace_scatter_shuffle<num_buckets>(vector_span, generator);

        for (std::size_t i = 0; i < size; i++) {
            for (std::size_t j = 0; j < size; j++) {
                if (i == j) {
                    continue;
                }
                std::size_t x = vector_span[i];
                std::size_t y = vector_span[j];
                std::size_t pair = x * size + y;
                std::size_t rank = i * size + j;
                results[pair][rank]++;
            }
        }
    }

    // my_print(results);

    for (std::size_t i = 0; i < m_size; i++) {
        for (std::size_t j = 0; j < m_size; j++) {
            // Excluding the rows and collumns where the numbers would be (x, x) and the indices (i, i).
            if ((i % (size + 1) == 0) ||  (j % (size + 1) == 0)) {
                continue;
            }
            // Number of p-tests = size * (size - 1) = m_size - size. As we exclude some rows and collumns.
            double p_value = calc_p_value(results[i][j], sample_size, 1.0 / static_cast<double>(m_size - size));
            bool reject = false;
            if (p_value < confidence / static_cast<double>(std::pow(m_size - size, 2))) {
                reject = true;
            }
            EXPECT_EQ(false, reject) << i << " " << j << " " << results[i][j];
        }
    }
}

/* 
INSTANTIATE_TEST_SUITE_P(CipShuffleTestUneven, 
                         CipShuffleTestFixture,
                         testing::Values(1, 3, 5, 7, 9, 11, 13, 15, 17, 19));

INSTANTIATE_TEST_SUITE_P(CipShuffleTestEven, 
                         CipShuffleTestFixture,
                         testing::Values(2, 4, 6, 8, 10, 12, 14, 16, 18, 20)); */

INSTANTIATE_TEST_SUITE_P(CipShuffleTest, 
                         CipShuffleTestFixture,
                         testing::Values(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 ,19, 20));
                         // 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 ,19, 20