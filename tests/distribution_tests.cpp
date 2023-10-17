#include <gtest/gtest.h>
#include <boost/math/distributions/chi_squared.hpp>

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

void my_print(std::vector<std::vector<std::size_t>> &matrix) {
    for (std::size_t i = 0; i < matrix.size(); i++) {
        for (std::size_t j = 0; j < matrix[i].size(); j++) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "\n";
    }
}

void my_print(std::vector<std::size_t>& vec) {
    for (auto& a : vec) {
        std::cout << a << " ";
    }
    std::cout << "\n";
}

//-------------------------------------------------------------------------------------------------

// We create a function which slices a generated number (64 bit) into multiple n bit numbers.
template<typename RNG>
void random_n_bit_numbners(int n, std::vector<std::uint64_t>& buffer, RNG &gen) {
    std::uint64_t bitmask = (1UL << n) - 1;

    std::uint64_t x = gen();
    for (std::size_t i = 0; i < buffer.size(); i++) {
        std::uint64_t chunk = static_cast<std::uint64_t>(x & bitmask); // Extract the lowest n bits
        buffer[i] = chunk;
        x = x >> n;
    }
}

//-------------------------------------------------------------------------------------------------

class DistributionTestFixture : public testing::TestWithParam<std::size_t> {
    protected:
        int seed;
        double confidence;

        void SetUp() override {
            seed = 6543976;
            confidence = 0.05;
        }
};


TEST_P(DistributionTestFixture, UniformIntDistr) {
    std::mt19937_64 generator(seed);
    std::size_t size = GetParam();
    std::size_t sample_size = 100 * size * size;
    std::vector<std::size_t> results(size);

    std::uniform_int_distribution<> distr(0, size - 1);
    for (std::size_t l = 0; l < sample_size; l++) {
        std::size_t i = distr(generator);
        results[i]++;
    }

    double critical_value = calc_critical_value(size - 1, confidence);
    double expected_value = static_cast<double>(sample_size) / static_cast<double>(size);
    double chi_squared_value = 0.0;
    for (size_t j = 0; j < size; j++) {
        chi_squared_value += std::pow(results[j] - expected_value, 2) / expected_value;
    }
    bool reject = (chi_squared_value > critical_value) ? true : false;
    EXPECT_EQ(false, reject) << chi_squared_value << " " << critical_value;
}

TEST_P(DistributionTestFixture, MyUniformIntDistr32) {
    std::mt19937_64 generator(seed);
    std::size_t size = GetParam();
    std::size_t sample_size = 100 * size * size;
    std::vector<std::size_t> results(size);

    for (std::size_t l = 0; l < sample_size; l++) {
        std::size_t i = my_uniform_int_distribution_32(size, generator);
        results[i]++;
    }

    double critical_value = calc_critical_value(size - 1, confidence);
    double expected_value = static_cast<double>(sample_size) / static_cast<double>(size);
    double chi_squared_value = 0.0;
    for (size_t j = 0; j < size; j++) {
        chi_squared_value += std::pow(results[j] - expected_value, 2) / expected_value;
    }
    bool reject = (chi_squared_value > critical_value) ? true : false;
    EXPECT_EQ(false, reject) << chi_squared_value << " " << critical_value;
}

TEST_P(DistributionTestFixture, MyUniformIntDistr64) {
    std::mt19937_64 generator(seed);
    std::size_t size = GetParam();
    std::size_t sample_size = 100 * size * size;
    std::vector<std::size_t> results(size);

    for (std::size_t l = 0; l < sample_size; l++) {
        std::size_t i = my_uniform_int_distribution_64(size, generator);
        results[i]++;
    }

    double critical_value = calc_critical_value(size - 1, confidence);
    double expected_value = static_cast<double>(sample_size) / static_cast<double>(size);
    double chi_squared_value = 0.0;
    for (size_t j = 0; j < size; j++) {
        chi_squared_value += std::pow(results[j] - expected_value, 2) / expected_value;
    }
    bool reject = (chi_squared_value > critical_value) ? true : false;
    EXPECT_EQ(false, reject) << chi_squared_value << " " << critical_value;
}


// Size only powers of 2
 TEST_P(DistributionTestFixture, MultipleUniformNumbers) {
    std::mt19937_64 generator(seed);
    // std::random_device rd;
    // std::mt19937_64 generator(rd());
    std::size_t size = GetParam();
    std::size_t sample_size = 100 * size * size;
    std::vector<std::size_t> results(size);

    int bits = static_cast<int>(std::log2(size));
    std::size_t buffer_size = 64 / bits;
    std::vector<std::uint64_t> buffer(buffer_size);

    // for (std::size_t l = 0; l < sample_size / buffer_size; l++) {
    //     random_n_bit_numbners(bits, buffer, generator);
    //     for (std::size_t j = 0; j < buffer_size; j++) {
    //         std::size_t i = buffer[j];
    //         results[i]++;
    //     }        
    // }

    std::size_t l = 0;
    std::size_t j = 0;
    random_n_bit_numbners(bits, buffer, generator);
    while (l < sample_size) {
        std::size_t i = buffer[j];
        results[i]++;

        j++;
        if (j >= buffer.size()) {
            random_n_bit_numbners(bits, buffer, generator);
            j = 0;
        }

        l++;
    }

    my_print(results);

    double critical_value = calc_critical_value(size - 1, confidence);
    double expected_value = static_cast<double>(sample_size) / static_cast<double>(size);
    double chi_squared_value = 0.0;
    for (size_t j = 0; j < size; j++) {
        chi_squared_value += std::pow(results[j] - expected_value, 2) / expected_value;
    }
    bool reject = (chi_squared_value > critical_value) ? true : false;
    EXPECT_EQ(false, reject) << chi_squared_value << " " << critical_value;
}

INSTANTIATE_TEST_SUITE_P(DistributionTest, 
                         DistributionTestFixture,
                         testing::Values(2, 4, 8, 16, 32, 64, 128, 256, 512, 1024));
