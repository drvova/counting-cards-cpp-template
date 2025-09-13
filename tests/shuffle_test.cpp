#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <cmath>
#include <iostream>
#include "../src/shuffle.hpp"

TEST_CASE("Shuffle Algorithms - Correctness Tests", "[shuffle]") {
    SECTION("All algorithms preserve array size") {
        std::vector<int> original = {1, 2, 3, 4, 5};
        
        std::vector<int> test1 = original;
        shuffle_random_sort(test1);
        REQUIRE(test1.size() == original.size());
        
        std::vector<int> test2 = original;
        shuffle_naive_swap(test2);
        REQUIRE(test2.size() == original.size());
        
        std::vector<int> test3 = original;
        shuffle_fisher_yates(test3);
        REQUIRE(test3.size() == original.size());
    }
    
    SECTION("All algorithms preserve array elements") {
        std::vector<int> original = {1, 2, 3, 4, 5};
        
        std::vector<int> test1 = original;
        shuffle_random_sort(test1);
        std::sort(test1.begin(), test1.end());
        REQUIRE(test1 == original);
        
        std::vector<int> test2 = original;
        shuffle_naive_swap(test2);
        std::sort(test2.begin(), test2.end());
        REQUIRE(test2 == original);
        
        std::vector<int> test3 = original;
        shuffle_fisher_yates(test3);
        std::sort(test3.begin(), test3.end());
        REQUIRE(test3 == original);
    }
    
    SECTION("Shuffles actually change order (high probability)") {
        std::vector<int> original(100);
        std::iota(original.begin(), original.end(), 0);
        
        bool changed = false;
        for (int trial = 0; trial < 10; ++trial) {
            std::vector<int> test = original;
            shuffle_fisher_yates(test);
            if (test != original) {
                changed = true;
                break;
            }
        }
        REQUIRE(changed);
    }
}

TEST_CASE("Shuffle Algorithms - Randomness Quality Tests", "[shuffle][randomness]") {
    const int TRIALS = 1000;
    const int ARRAY_SIZE = 52; // Standard deck size
    
    auto test_distribution = [&](const std::string& name, auto shuffle_func) {
        std::unordered_map<int, int> position_counts;
        
        for (int trial = 0; trial < TRIALS; ++trial) {
            std::vector<int> deck(ARRAY_SIZE);
            std::iota(deck.begin(), deck.end(), 0);
            shuffle_func(deck);
            
            for (int pos = 0; pos < ARRAY_SIZE; ++pos) {
                position_counts[deck[pos] * ARRAY_SIZE + pos]++;
            }
        }
        
        double expected = static_cast<double>(TRIALS) / ARRAY_SIZE;
        double chi_squared = 0.0;
        
        for (const auto& [key, count] : position_counts) {
            double diff = count - expected;
            chi_squared += (diff * diff) / expected;
        }
        
        double degrees_of_freedom = ARRAY_SIZE * ARRAY_SIZE - 1;
        double max_chi_squared = degrees_of_freedom * 2.0; // Rough threshold
        
        REQUIRE(chi_squared < max_chi_squared);
    };
    
    SECTION("Fisher-Yates produces uniform distribution") {
        test_distribution("fisher_yates", shuffle_fisher_yates);
    }
}

TEST_CASE("Shuffle Algorithms - Performance Benchmarks", "[shuffle][benchmark]") {
    const std::vector<size_t> test_sizes = {10, 100, 1000, 10000};
    
    for (size_t size : test_sizes) {
        std::vector<int> test_data(size);
        std::iota(test_data.begin(), test_data.end(), 0);
        
        SECTION("Performance with size " + std::to_string(size)) {
            BENCHMARK("Random Sort (size=" + std::to_string(size) + ")") {
                std::vector<int> copy = test_data;
                shuffle_random_sort(copy);
                return copy;
            };
            
            BENCHMARK("Naive Swap (size=" + std::to_string(size) + ")") {
                std::vector<int> copy = test_data;
                shuffle_naive_swap(copy);
                return copy;
            };
            
            BENCHMARK("Fisher-Yates (size=" + std::to_string(size) + ")") {
                std::vector<int> copy = test_data;
                shuffle_fisher_yates(copy);
                return copy;
            };
        }
    }
}

TEST_CASE("Shuffle Algorithms - Multiple Trials Analysis", "[shuffle][trials]") {
    const std::vector<size_t> array_sizes = {10, 100, 1000, 10000};
    const int trials_per_size = 4;
    
    for (size_t array_size : array_sizes) {
        SECTION("Testing with array size " + std::to_string(array_size)) {
            std::vector<double> random_sort_times;
            std::vector<double> naive_swap_times;
            std::vector<double> fisher_yates_times;
            
            for (int trial = 0; trial < trials_per_size; ++trial) {
                std::vector<int> test_data(array_size);
                std::iota(test_data.begin(), test_data.end(), 0);
                
                auto measure_time = [](auto func, std::vector<int>& data) {
                    auto start = std::chrono::high_resolution_clock::now();
                    func(data);
                    auto end = std::chrono::high_resolution_clock::now();
                    return std::chrono::duration<double, std::milli>(end - start).count();
                };
                
                std::vector<int> copy1 = test_data;
                double time1 = measure_time(shuffle_random_sort, copy1);
                random_sort_times.push_back(time1);
                
                std::vector<int> copy2 = test_data;
                double time2 = measure_time(shuffle_naive_swap, copy2);
                naive_swap_times.push_back(time2);
                
                std::vector<int> copy3 = test_data;
                double time3 = measure_time(shuffle_fisher_yates, copy3);
                fisher_yates_times.push_back(time3);
            }
            
            auto calculate_average = [](const std::vector<double>& times) {
                return std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            };
            
            double avg_random_sort = calculate_average(random_sort_times);
            double avg_naive_swap = calculate_average(naive_swap_times);
            double avg_fisher_yates = calculate_average(fisher_yates_times);
            
            std::cout << "\nArray Size: " << array_size << "\n";
            std::cout << "Random Sort avg: " << avg_random_sort << " ms\n";
            std::cout << "Naive Swap avg: " << avg_naive_swap << " ms\n";
            std::cout << "Fisher-Yates avg: " << avg_fisher_yates << " ms\n";
            
            REQUIRE(avg_fisher_yates < avg_random_sort);
            REQUIRE(avg_fisher_yates < avg_naive_swap);
        }
    }
}

TEST_CASE("Shuffle Algorithms - Edge Cases", "[shuffle][edge]") {
    SECTION("Empty array") {
        std::vector<int> empty;
        REQUIRE_NOTHROW(shuffle_random_sort(empty));
        REQUIRE_NOTHROW(shuffle_naive_swap(empty));
        REQUIRE_NOTHROW(shuffle_fisher_yates(empty));
        REQUIRE(empty.empty());
    }
    
    SECTION("Single element array") {
        std::vector<int> single = {42};
        std::vector<int> original = single;
        
        shuffle_random_sort(single);
        REQUIRE(single == original);
        
        single = {42};
        shuffle_naive_swap(single);
        REQUIRE(single == original);
        
        single = {42};
        shuffle_fisher_yates(single);
        REQUIRE(single == original);
    }
    
    SECTION("Two element array") {
        std::vector<int> two = {1, 2};
        
        int original_first_count = 0;
        int swaps = 1000;
        
        for (int i = 0; i < swaps; ++i) {
            std::vector<int> test = two;
            shuffle_fisher_yates(test);
            if (test[0] == 1) original_first_count++;
        }
        
        double probability = static_cast<double>(original_first_count) / swaps;
        REQUIRE((probability > 0.4 && probability < 0.6));
    }
}