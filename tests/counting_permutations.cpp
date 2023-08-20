#include <gtest/gtest.h>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/math/special_functions/factorials.hpp>
#include <map>

#include <cip_shuffle.hpp>

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

void my_print(std::map<std::vector<size_t>, size_t> &map) {
    for (auto const& pair : map) {
        auto keys = pair.first;
        for (auto const& v : keys) {
            std::cout << v << " ";
        }
        std::cout << ": " << pair.second << "\n";
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

    // std::size_t param = GetParam();
    const std::size_t size = GetParam();
    const std::size_t size_factorial = static_cast<size_t>(boost::math::factorial<double>(size));

    std::size_t sample_size = 100 * size_factorial * static_cast<size_t>(std::ceil(std::log(size_factorial)));

    std::map<std::vector<size_t>, size_t> result_map;

    for (std::size_t l = 0; l < sample_size; l++) {
        std::vector<std::size_t> vec(size);
        std::iota(vec.begin(), vec.end(), 0);
        std::span vector_span {vec};
        inplace_scatter_shuffle<num_buckets>(vector_span, generator);

        if (result_map.contains(vec)) {
            result_map[vec]++;
        } else {
            result_map.insert({vec, 1});
        }
    }

    // We check if we got every permutation, else we can stop here.
    ASSERT_EQ(size_factorial, result_map.size());

    double critical_value = calc_critical_value(size_factorial - 1, confidence);
    double expected_value = static_cast<double>(sample_size) / static_cast<double>(size_factorial);
    double chi_squared_value = 0.0;
    for (auto const& pair : result_map) {
        chi_squared_value += std::pow(pair.second - expected_value, 2) / expected_value;
    }
    bool reject = (chi_squared_value > critical_value) ? true : false;
    EXPECT_EQ(false, reject) << chi_squared_value << " " << critical_value;

    /* my_print(result_map);
    std::cout << result_map.size() << "\n";
    EXPECT_EQ(false, true) << "ignore this"; */
}

INSTANTIATE_TEST_SUITE_P(CipShuffleTest, 
                         CipShuffleTestFixture,
                         testing::Values(4, 5, 6, 7, 8));