#include <gtest/gtest.h>
#include <boost/math/distributions/chi_squared.hpp>

#include <cip_shuffle.hpp>
#include "pcg-cpp-0.98/include/pcg_random.hpp"

double calc_critical_value(int degree_of_freedom, double alpha) {
    try {
        boost::math::chi_squared distr(degree_of_freedom);
        // This gives us the upper critical value to the distribution. In other words, 
        // it returns x such that P(X > x) == confidence. 
        double critical_value = quantile(complement(distr, alpha));
        return critical_value;
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
            seed = 1234567;
            confidence = 0.05;
        }
};

TEST_P(CipShuffleTestFixture, IndependenceTest) {
    // std::mt19937_64 generator(seed);
    pcg64 generator(seed);

    std::size_t param = GetParam();
    const std::size_t size = param;

    std::size_t sample_size = 1000 * size * size;
    std::vector<std::vector<std::size_t>> results(size, std::vector<std::size_t>(size));

    for (std::size_t l = 0; l < sample_size; l++) {
        std::vector<std::size_t> V(size);
        std::iota(V.begin(), V.end(), 0);
        std::span vector_span {V};
        // inplace_scatter_shuffle(vector_span, generator);
        // buffered_fisher_yates_shuffle_64(vector_span, generator);
        fisher_yates_shuffle_64(vector_span, generator);

        for (std::size_t j = 0; j < size; j++) {
            std::size_t i = vector_span[j];
            results[i][j]++;
        }
    }

    my_print(results);

    double critical_value = calc_critical_value(size - 1, confidence / static_cast<double>(size));
    double expected_value = static_cast<double>(sample_size) / static_cast<double>(size);
    for (size_t i = 0; i < size; i++) {
        std::vector<size_t> observations = results[i];
        double chi_squared_value = 0.0;
        for (size_t j = 0; j < size; j++) {
            chi_squared_value += std::pow(observations[j] - expected_value, 2) / expected_value;
        }
        bool reject = (chi_squared_value > critical_value) ? true : false;
        EXPECT_EQ(false, reject) << i << " " << chi_squared_value << " " << critical_value;
    }
}

INSTANTIATE_TEST_SUITE_P(CipShuffleTest, 
                         CipShuffleTestFixture,
                         testing::Values(10, 20, 30, 40, 50, 60, 70, 80, 90, 100));