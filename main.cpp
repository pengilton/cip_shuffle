#include <iostream>
#include <random>
#include <array>
#include <vector>
#include <chrono>
#include <span>

// Bucket as data structure
struct bucket {
    uint64_t b_i;
    uint64_t s_i;
    uint64_t e_i;
};


// Fisher Yates
template<typename T>
void fisher_yates_shuffle(std::vector<T> &vec, std::default_random_engine &gen) {
    for (int i = 0; i < vec.size() - 1; i++) {
        // uniform sample from [i, N)
        std::uniform_int_distribution<> distrib(i, vec.size()-1);
        int j = distrib(gen);

        // swapping of values -> might change this with std::swap
        using std::swap;
        swap(vec[i], vec[j]);
    }
}

// Fisher-Yates
// Swapping of elements could be its own function. Maybe there is a way to swap elements
// in the standard library 
template<typename T>
void fisher_yates_shuffle(std::span<T> data_span, std::default_random_engine &gen) {
    for (int i = 0; i < data_span.size() - 1; i++) {
        // uniform sample from [i, N)
        std::uniform_int_distribution<> distrib(i, data_span.size()-1);
        int j = distrib(gen);

        // swapping of values -> might change this with std::swap
        using std::swap;
        swap(data_span[i], data_span[j]);
    }
}

// There might be a better way than this. I only want to referene a subvector. 
// Maybe iterators? For now, this should do the trick.
template<typename T>
void fisher_yates_shuffle(std::span<T> data_span, int size, std::default_random_engine &gen) {
    for (int i = 0; i < size - 1; i++) {
        // uniform sample from [i, N)
        std::uniform_int_distribution<> distrib(i, size-1);
        int j = distrib(gen);

        // swapping of values -> might change this with std::swap
        using std::swap;
        swap(data_span[i], data_span[j]);
    }
}


// Rough Scatter
// Might have to swap size_t and typename
template<std::size_t K, typename T>
void rough_scatter(std::span<T> data_span, std::array<bucket, K> &buckets, std::default_random_engine &gen) {
    // Uniform distribution from 0 to k-1
    std::uniform_int_distribution<> distrib(0, K-1);


    // Lets remove bool = done and add if break
    while (true) {
        int j = distrib(gen);

        // Not sure if I need all those variables.
        int s_0 = buckets[0].s_i;
        int s_j = buckets[j].s_i;

        // Benchmark if this if slows down the code
        if (j != 0) {
            // They are helpful here
            using std::swap;
            swap(data_span[s_0], data_span[s_j]);
        }

        // Maybe just use the increment operator directly
        s_j++;
        buckets[j].s_i = s_j;

        if (buckets[j].s_i == buckets[j].e_i) {
            break;
        }
    }
}

// Fine Scatter. I might have to implement that sweaping thing as a separate function
template<std::size_t K, typename T>
void fine_scatter(std::span<T> data_span, std::array<bucket, K> &buckets, std::default_random_engine &gen) {
    std::vector<int> n_placed(K);           // ith element = #placed items in bucket i 
    std::vector<int> n_to_be_placed(K);     // ith element = #items that will be placed in bucket i; multinomial
    std::vector<int> n_f(K);                // ith element = size of bucket i
    std::vector<int> d(K);                  // ith element = n_f_i - n/k
    std::vector<int> c(K, 0);                  // ci indicates how many items are needed left of bucket i

    int num_staged_items = 0;

    // Calcualtes the number of palced items in each bucket and store the 
    // result in n_placed
    for (int i = 0; i < K; i++) {
        // si - bi
        n_placed[i] = buckets[i].s_i - buckets[i].b_i;

        // While we are iterating over the elements, we can also get the number of 
        // items whhich are still staged. 
        // e_i - s_i
        num_staged_items += buckets[i].e_i - buckets[i].s_i;
    }

    // Technically this line could be replaced by prob(k, 1) as 
    // discrete_distribution already divides each element by its sum.
    std::vector<double> prob(K, 1.0/K);
    std::discrete_distribution<> distrib(prob.begin(), prob.end());
    // I followed the example in the documentation (cppreference)
    for (int i = 0; i < num_staged_items; i++) {
        n_to_be_placed[distrib(gen)]++;
    }

    // Calculate the sum of both vectors and store it in n_f
    // Here I could use the std::transform function
    for (int i = 0; i < K; i++) {
        n_f[i] = n_placed[i] + n_to_be_placed[i];
    }

    // Now we calculate all di
    for (int i = 0; i < K; i++) {
        // d[i] = n_f[i] - data_span.size() / K;

        // I use the actual bucket size instead of the mean
        d[i] = n_f[i] - (buckets[i].e_i - buckets[i].b_i);
    }

    // Now we fill the vector c
    // Think I can make this better because c[i] = c[i-1] + d[i-1]
    for (int i = 1; i < K; i++) {
        for (int j = 0; j <= i-1; j++) {
            c[i] += d[j];
        }
    }

    // Now we can do the TwoSweep part 
    // We sweep from left to right
    for (int i = 0; i < K-1; i++) {
        // If bucket Bi is too large by more than ci we move excess staged items into
        // the staging area of bucket i+1
        // NOTE: buckets store uint64_t data and everything else is an int! Might cause issues!
        while ((buckets[i].e_i - buckets[i].b_i) > c[i] + n_f[i]) {
            // Swapping the last staged item of bucket i with the last placed item of bucket i+1
            using std::swap;
            swap(data_span[buckets[i].e_i - 1], data_span[buckets[i+1].s_i - 1]);
            // Adjusting the buckets
            buckets[i].e_i--;
            buckets[i+1].b_i--;
            buckets[i+1].s_i--;
        }
    }

    // We sweep from right to left
    for (int i = K-1; i > 0; i--) {
        while (buckets[i].e_i - buckets[i].b_i > n_f[i]) {
            // Swapping first placed item in bucket i with the first staged item in bucket i
            using std::swap;
            swap(data_span[buckets[i].b_i], data_span[buckets[i].s_i]);
            // Adjusting the buckets
            buckets[i].s_i++;
            buckets[i].b_i++;
            buckets[i-1].e_i++;
        }
    }

    // Now we can assign the remaining staged items. We move the staged
    // items together.
    std::vector<int> stack;
    int i = 0;
    for (int j = 0; j < K; j++) {
        int l = buckets[j].s_i;
        while (l < buckets[j].e_i) {
            using std::swap;
            swap(data_span[i], data_span[l]);
            stack.push_back(l);
            i++;
            l++;
        }
    }

    // All staged items should be grouped together now. Using Fisher-Yates here
    // Maybe use subspan here instad of using stack.size()
    fisher_yates_shuffle(data_span, stack.size(), gen);

    // Now reverting the reordering
    while (stack.size() > 0) {
        i--;
        using std::swap;
        swap(data_span[i], data_span[stack.back()]);
        stack.pop_back();
    }
}


// We will implenet the sequential version of scatter shuffle
template<typename T>
void scatter_shuffle(std::vector<T> &data_span, int k, std::default_random_engine &gen) {
    // Might be unnecessary 
    if (data_span.empty()) {
        return;
    }

    int small = 256;
    if (data_span.size() <= small) {
        fisher_yates_shuffle(data_span, gen);
        return;
    }

    // We have k buckets. A bucket is a vecotr for now. 
    std::vector<T> buckets[k];
    // Uniform distribution from 0 to k-1
    std::uniform_int_distribution<> distrib(0, k-1);

    for (int i = 0; i < data_span.size(); i++) {
        // randomly choose a bucket
        int j = distrib(gen);
        // copies value to chosen bucket
        buckets[j].push_back(data_span[i]);
    }

    int s = 0;
    for (int j = 0; j < k; j++) {
        scatter_shuffle(buckets[j], k, gen);
        std::copy(buckets[j].begin(), buckets[j].end(), data_span.begin() + s);
        s += buckets[j].size();
    }
}

// IpScShuf
// First draft of the inplace scatter shuffle algorithm based on chapter 3
template<std::size_t K, typename T>
void inplace_scatter_shuffle(std::span<T> data_span, std::default_random_engine &gen) {
    // Might be unnecessary
    if (data_span.empty()) {
        return;
    }

    // Change to 256
    int small = 2;
    if (data_span.size() <= small) {
        fisher_yates_shuffle(data_span, gen);
        return;
    }

    // An vector which contains the triples of indices (bi, si, ei) but instead of having an 
    // array of triples we just put them next to each other. Hence each block of three elements in 
    // the array is one triple.
    // Maybe I should creade a small fucntion which creates the buckets to make this function 
    // a bit cleaner.
    std::array<bucket, K> buckets;
    for (int i = 0; i < K; i++) {
        buckets[i].b_i = static_cast<uint64_t>(data_span.size()) * i / K;
        buckets[i].s_i = static_cast<uint64_t>(data_span.size()) * i / K;;
        buckets[i].e_i = static_cast<uint64_t>(data_span.size()) * (i+1) / K;
    }

    // Rough Scatter
    rough_scatter<K>(data_span, buckets, gen);

    // Fine Scatter which does the twosweap thing and assigns the last itmes 
    fine_scatter<K>(data_span, buckets, gen);

    // Squentially calling inplace_scatter_schuffle on each bucket.
    // This part should be hhighly parallelisable.
    for (int i = 0; i < K; i++) {
        // Might be unnecessary to create a span
        std::span bucket_span = data_span.subspan(buckets[i].b_i, buckets[i].e_i - buckets[i].b_i);
        inplace_scatter_shuffle<K>(bucket_span, gen);
    }
}


// Function to print stuff
template<typename T>
void print_vec(std::vector<T> &vec) {
    int numbers_per_line = 16;
    for (int i = 0; i < vec.size(); i++) {
        if ((i % numbers_per_line == 0) && (i != 0)) {
            std::cout << std::endl;
        }
        std::cout << vec[i] << ", ";
    }
    std::cout << std::endl;
}


int main(int argc, char const *argv[]) {
    // Rabndom generator
    std::random_device rd;
    // use fixed seed to test
    // int seed = 0;
    int seed = rd();
    std::default_random_engine generator(seed);

    // vector size
    int size = 1000000;
    // amound of buckets
    constexpr std::size_t K = 16;

    /* // runs
    int runs = 10;

    for (int i = 0; i < runs; i++) {
        std::vector<int> V(size);
        std::iota(V.begin(), V.end(), 0);
        
        std::span span = std::span(V);

        auto start = std::chrono::steady_clock::now();
        // scatter_shuffle(V, k, generator);
        inplace_scatter_shuffle<K>(span, generator);
        auto end = std::chrono::steady_clock::now();

        // print array after shuffle
        // print_vec(V);

        std::chrono::duration<double> elapsed_time = end - start;
        std::cout << "Runtime: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count() 
                  << "ms" << std::endl;
    } */

    //-------------------------------------------

    // Let's do some testing if code is correct
    int sample_size = 10;
    std::vector<int> values(sample_size, 0);
    int runs = 1;

    for (int i = 0; i < runs; i++) {
        std::vector<int> V(size);
        std::iota(V.begin(), V.end(), 0);
        
        std::span span = std::span(V);

        inplace_scatter_shuffle<K>(span, generator);

        for (int j = 0; j < sample_size; j++) {
            values[j] += V[j];
        }
    }

    std::cout << "";

    for (int i = 0; i < sample_size; i++) {
        std::cout << values[i] << " ";
    } 
    std::cout << std::endl;

    return 0;
}
