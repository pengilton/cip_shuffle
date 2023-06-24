#include <iostream>
#include <random>
#include <array>
#include <vector>
#include <chrono>

// Fisher-Yates
// Swapping of elements could be its own function. Maybe there is a way to swap elements
// in the standard library 
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

// We will implenet the sequential version of scatter shuffle
template<typename T>
void scatter_shuffle(std::vector<T> &vec, int k, std::default_random_engine &gen) {
    // Might be unnecessary 
    if (vec.size() == 0) {
        return;
    }

    int small = 256;
    if (vec.size() <= small) {
        fisher_yates_shuffle(vec, gen);
        return;
    }

    // We have k buckets. A bucket is a vecotr for now. 
    std::vector<T> buckets[k];
    // Uniform distribution from 0 to k-1
    std::uniform_int_distribution<> distrib(0, k-1);

    for (int i = 0; i < vec.size(); i++) {
        // randomly choose a bucket
        int j = distrib(gen);
        // copies value to chosen bucket
        buckets[j].push_back(vec[i]);
    }

    int s = 0;
    for (int j = 0; j < k; j++) {
        scatter_shuffle(buckets[j], k, gen);
        std::copy(buckets[j].begin(), buckets[j].end(), vec.begin() + s);
        s += buckets[j].size();
    }
}


// IpScShuf
// First draft of the inplace scatter shuffle algorithm based on chapter 3
template<typename T>
void inplace_scatter_shuffle(std::vector<T> &vec, int k, std::default_random_engine &gen) {
    // Might be unnecessary
    if (vec.size() == 0) {
        return;
    }

    int small = 256;
    if (vec.size() <= small) {
        fisher_yates_shuffle(vec, gen);
        return;
    }

    // An vector which contains the triples of indices (bi, si, ei) but instead of having an 
    // array of triples we just put them next to each other. Hence each block of three elements in 
    // the array is one triple.
    // Maybe I should creade a small fucntion which creates the buckets to make this function 
    // a bit cleaner.
    std::vector<int> buckets(3*k);
    int b_size = vec.size() / k;
    int remainder = vec.size() % k;
    int p = 0;
    for (int i = 0; i < k; i++) {
        buckets[3*i] = p;    // bi
        buckets[3*i+1] = p;  // si
        p += b_size;
        if (remainder > 0) {
            p++;
            remainder--;
        }
        buckets[3*i+2] = p; // ei
    }

    // Rough Scatter
    rough_scatter(vec, buckets, k, gen);

    // Fine Scatter which does the twosweap thing and assigns the last itmes 
    fine_scatter(vec, buckets, k, gen);

    // same code as in ScShuf
    // here it breaks.. I cannot pass a part of a vector 
    int s = 0;
    for (int j = 0; j < k; j++) {
        scatter_shuffle(buckets[j], k, gen);
        std::copy(buckets[j].begin(), buckets[j].end(), vec.begin() + s);
        s += buckets[j].size();
    }
}

// Rough Scatter
template<typename T>
void rough_scatter(std::vector<T> &vec, std::vector<int> &buckets, int k, std::default_random_engine &gen) {
    // Uniform distribution from 0 to k-1
    std::uniform_int_distribution<> distrib(0, k-1);

    bool done = false;
    while (!done) {
        int j = distrib(gen);

        // Not sure if I need all those variables.
        int s_0 = buckets[1];
        int s_j = buckets[3*j+1];

        if (j != 0) {
            // They are helpful here
            using std::swap;
            swap(vec[s_0], vec[s_j]);
        }

        s_j++;
        buckets[3*j+1] = s_j;

        if (buckets[3*j+1] == buckets[3*j+2]) {
            done = true;
        }
    }
}

// Fine Scattr. I might have to implement that sweaping thing as a separate function
template<typename T>
void fine_scatter(std::vector<T> &vec, std::vector<int> &buckets, int k, std::default_random_engine &gen) {
    std::vector<int> n_placed(k);   // ith element = #placed items in bucket i
    std::vector<int> n_staged(k);   // ith element = #items that will be placed in bucket i; multinomial
    std::vector<int> n_f(k);        // ith element = size of bucket i
    std::vector<int> d(k);          // ith element = n_f_i - n/k
    std::vector<int> c(k);          // ci indicates how many items are needed left of bucket i

    int num_staged_items = 0;

    // Calcualtes the number of palced items in each bucket and store the 
    // result in n_placed
    for (int i = 0; i < k; i++) {
        // si - bi
        n_placed[i] = buckets[3*i+1] - buckets[3*i] 

        // While we are iterating over the elements, we can also get the number of 
        // items whhich are still staged. 
        num_staged_items += buckets[3*i+2] - buckets[3*i+1]
    }

    // Technically this line could be replaced by prob(k, 1) as 
    // discrete_distribution already divides each element by its sum.
    std::vector<double> prob(k, 1.0/k);
    std::discrete_distribution<> distrib(prob.begin(), prob.end());
    // I followed the example in the documentation (cppreference)
    for (int i = 0; i < num_staged_items; i++) {
        n_staged[d(gen)]++;
    }

    // Calculate the sum of both vectors and store it in n_f
    // Here I could use the std::transform function
    for (int i = 0; i < k; i++) {
        n_f[i] = n_placed[i] + n_staged[i];
    }

    // Now we calculate all di
    for (int i = 0; i < k; i++) {
        // This line is problematic. What if vec.size() is not divisable by k? 
        // One idea: rounding 
        // Another idea: keep them as floats/doubles?
        // for now I assume that vec.size() is divisabel by k
        d[i] = n_f[i] - vec.size() / k;
    }

    // Now we fill the vector c
    // Think I can make this better because c[i] = c[i-1] + d[i-1]
    for (int i = 1; i < k; i++) {
        for (int j = 0; j < i-1; j++) {
            c[i] += d[j];
        }
    }

    // Now we can do the TwoSweep part 
    // We sweep from left to right
    for (int i = 0; i < k-1; i++) {
        // If bucket Bi is too large by more than ci we move excess staged items into
        // the staging area of 
        while ((buckets[3*i+2] - buckets[3*i]) - n_f[i] > c[i]) {
            // Swapping the last placed item of bucket i+1 with the last staged item of bucket i
            using std::swap;
            swap(vec[buckets[3*i+2] - 1] vec[buckets[3*(i+1)+1] - 1]);
            // Adjusting the buckets
            buckets[3*i+2]--;
            buckets[3*(i+1)]--;
            buckets[3*(i+1)+1]--;
        }
    }

    // We sweep from right to left
    for (int i = k-1; i > 0; i--) {
        while (buckets[3*i+2] - buckets[3*i] > n_f[i]) {
            // Swapping first placed item in bucket i with the first staged item in bucket i
            using std::swap;
            swap(vec[buckets[3*i]], vec[buckets[3*i+1]]);
            // Adjusting the buckets
            buckets[3*i+1]++;
            buckets[3*i]++;
            buckets[3*(i-1)+2]++;
        }
    }

    // Now we can assign the remaining staged items
    std::vector<int> stack;
    int i = 0;
    for (int j = 0; j < k; i++) {
        int l = buckets[3*j+1]
        while (l < buckets[3*j+2]) {
            using std::swap;
            swap(vec[i], vec[l]);
            stack.push_back(l);
            i++;
            l++;
        }
    }

    // All staged items should be grouped together now. Using Fisher-Yates here
    fisher_yates_shuffle(vec, stack.size(), gen);

    // Now reverting the reordering
    while (stack.size() > 0) {
        i--;
        using std::swap;
        swap(vec[i], vec[stack.back()]);
        vec.pop_back();
    }
}

// There might be a better way than this. I only want to referene a subvector. 
// Maybe iterators? For now, this should do the trick.
template<typename T>
void fisher_yates_shuffle(std::vector<T> &vec, int size, std::default_random_engine &gen) {
    for (int i = 0; i < size - 1; i++) {
        // uniform sample from [i, N)
        std::uniform_int_distribution<> distrib(i, size-1);
        int j = distrib(gen);

        // swapping of values -> might change this with std::swap
        using std::swap;
        swap(vec[i], vec[j]);
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
    int k = 16;
    // runs
    int runs = 10;

    for (int i = 0; i < runs; i++) {
        std::vector<int> V(size);
        std::iota(V.begin(), V.end(), 0);

        auto start = std::chrono::steady_clock::now();
        scatter_shuffle(V, k, generator);
        auto end = std::chrono::steady_clock::now();

        // print array after shuffle
        // print_vec(V);

        std::chrono::duration<double> elapsed_time = end - start;
        std::cout << "Runtime: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count() 
                  << "ms" << std::endl;
    }

    return 0;
}
