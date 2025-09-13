#pragma once

#include <vector>
#include <random>
#include <algorithm>

// Algorithm A: Random sort (biased - worst)
void shuffle_random_sort(std::vector<int>& array);

// Algorithm B: Naive swap (biased - middle)  
void shuffle_naive_swap(std::vector<int>& array);

// Algorithm C: Fisher-Yates (unbiased - best)
void shuffle_fisher_yates(std::vector<int>& array);

