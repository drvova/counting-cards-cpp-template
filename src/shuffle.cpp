#include "shuffle.hpp"
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_set>
#include <chrono>

namespace {
    std::mt19937 rng;
    bool rng_initialized = false;
}

std::mt19937& get_rng() {
    if (!rng_initialized) {
        auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
        rng.seed(seed);
        rng_initialized = true;
    }
    return rng;
}

void shuffle_random_sort(std::vector<int>& array) {
    if (array.empty()) return;
    
    std::vector<int> result;
    std::unordered_set<int> used_indices;
    auto& rng = get_rng();
    std::uniform_int_distribution<size_t> dist(0, array.size() - 1);
    
    while (result.size() < array.size()) {
        size_t random_index = dist(rng);
        if (used_indices.insert(random_index).second) {
            result.push_back(array[random_index]);
        }
    }
    
    array = result;
}

void shuffle_naive_swap(std::vector<int>& array) {
    if (array.empty()) return;
    
    auto& rng = get_rng();
    std::uniform_int_distribution<size_t> dist(0, array.size() - 1);
    
    for (size_t i = 0; i < array.size(); ++i) {
        size_t random_index = dist(rng);
        std::swap(array[i], array[random_index]);
    }
}

void shuffle_fisher_yates(std::vector<int>& array) {
    if (array.empty()) return;
    
    auto& rng = get_rng();
    
    for (size_t i = array.size() - 1; i > 0; --i) {
        size_t random_index = rng() % (i + 1);
        std::swap(array[i], array[random_index]);
    }
}
