
#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <vector>
#include <iostream>
#include "unq_ptr.h"
#include "shrd_ptr.h"

template <class Func>
long long measure_time(Func function) {
    auto start = std::chrono::steady_clock::now();
    function();
    auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        finish - start
    ).count();
}

TEST(BenchmarkTest, CreationAndDestruction) {
    const int count = 100000;
    long long raw_time = measure_time([&]() {
        std::vector<int*> values;
        values.reserve(count);
        for (int i = 0; i < count; i++) {
            values.push_back(new int(i));
        }
        for (int* value : values) {
            delete value;
        }
    });
    long long std_unique_time = measure_time([&]() {
        std::vector<std::unique_ptr<int>> values;
        values.reserve(count);
        for (int i = 0; i < count; i++) {
            values.push_back(std::make_unique<int>(i));
        }
    });
    long long unq_time = measure_time([&]() {
        std::vector<UnqPtr<int>> values;
        values.reserve(count);
        for (int i = 0; i < count; i++) {
            values.emplace_back(new int(i));
        }
    });
    long long std_shared_time = measure_time([&]() {
        std::vector<std::shared_ptr<int>> values;
        values.reserve(count);
        for (int i = 0; i < count; i++) {
            values.push_back(std::make_shared<int>(i));
        }
    });
    long long shrd_time = measure_time([&]() {
        std::vector<ShrdPtr<int>> values;
        values.reserve(count);
        for (int i = 0; i < count; i++) {
            values.emplace_back(new int(i));
        }
    });
    std::cout << "\nCreation and destruction of " << count << " objects:\n";
    std::cout << "Raw pointer:       " << raw_time << " us\n";
    std::cout << "std::unique_ptr:   " << std_unique_time << " us\n";
    std::cout << "UnqPtr:           " << unq_time << " us\n";
    std::cout << "std::shared_ptr:   " << std_shared_time << " us\n";
    std::cout << "ShrdPtr:          " << shrd_time << " us\n";
    EXPECT_GE(raw_time, 0);
    EXPECT_GE(std_unique_time, 0);
    EXPECT_GE(unq_time, 0);
    EXPECT_GE(std_shared_time, 0);
    EXPECT_GE(shrd_time, 0);
}
