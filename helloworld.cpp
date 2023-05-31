#include <iostream>
#include <vector>
#include <string>

using namespace std;

template<typename T>
void foo(vector<T> &vec) {
    for (int i = 0; i < vec.size(); i++) {
        cout << vec[i] << " ";
    }
    cout << endl;
}

template<typename T>
void food_dumb(vector<T> vec) {
    vec[0] = 100;
    for (int i = 0; i < vec.size(); i++) {
        cout << vec[i] << " ";
    }
    cout << endl;
}

int main() {
    vector<string> msg{"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};
    for (const string &word: msg) {
        cout << word << " ";
    }
    cout << endl;

    vector<int> numbers = {1,2,3,4,5};
    vector<string> text = {"Hello", "World!"};
    
    cout << "&numbers: " << &numbers << endl;
    foo(numbers);
    cout << "Lets change locally smth with foo_dumb" << endl;
    food_dumb(numbers);
    foo(numbers);

    // int num[5] = {523, 458, 426, 421, 123};
    // int *numbers[5];
    // for (int i = 0; i < 5; i++) {
    //     numbers[i] = &num[i];
    // }

    // for (int i = 0; i < 5; i++) {
    //     std::cout << "numbers: " << numbers[i] << ": ";
    //     std::cout << "num: " << num[i] << "; "; 
    //     std::cout << "*numbers[i]: " << *numbers[i] << std::endl;
    // }

    // int k = 2;
    // std::vector<int> *buckets[k];
    // for (int i = 0; i < k; i++) {
    //     std::vector<int> bucket = {};
    //     buckets[i] = &bucket;
    // }
    // buckets[0]->push_back(1);
    // buckets[1]->push_back(10);
    // buckets[1]->push_back(20);
    // buckets[0]->push_back(2);
    // buckets[0]->push_back(3);
    // for (int i = 0; i < k; i++) {
    //     for (int j = 0; j < buckets[i]->size(); j++) {
    //         std::cout << buckets[i]->at(j) << std::endl;
    //     }
    // }

    return 0;
}