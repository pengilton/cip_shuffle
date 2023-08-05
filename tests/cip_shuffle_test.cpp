#include <gtest/gtest.h>

#include <cip_shuffle.hpp>

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
