#include <iostream>
#include <random>
#include <array>

// Fisher-Yates
// Does only work with integers right now
// template<typename T, size_t N>
void fisher_yates_shuffle(int arr[], int size, std::default_random_engine &gen) {
    for (int i = 0; i < size; i++) {
        // uniform sample from [i, N-1]
        std::uniform_int_distribution<> distrib(i, size-1);
        int j = distrib(gen);

        // swapping of values
        auto temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// We will implenet the sequential version of scatter shuffle
void scatter_shuffle(int arr[], int size, int k, std::default_random_engine &gen) {
    int small = 32;
    if (size <= small) {
        fisher_yates_shuffle(arr, size, gen);
    }

    
}

// Function to print stuff
void print_array(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        std::cout << arr[i] << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, char const *argv[]) {
    // Rabndom generator
    std::random_device rd;
    std::default_random_engine generator(rd());

    // arbitary array
    int size = 10;
    int A[size];
    for (int i = 0; i < size; i++) {
        A[i] = i;
    }

    // print array before shuffle
    print_array(A, size);

    fisher_yates_shuffle(A, size, generator);

    // print array after shuffle
    print_array(A, size);

    return 0;
}
