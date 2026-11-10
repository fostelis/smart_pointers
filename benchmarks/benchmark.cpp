
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "unq_ptr.h"
#include "shrd_ptr.h"

template <class Function>
double measure_time(Function function) {
    auto start = std::chrono::steady_clock::now();
    function();
    auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(finish - start).count();
}

template <class Function>
double measure_median(Function function, int repetitions) {
    std::vector<double> times;
    times.reserve(repetitions);
    for (int i = 0; i < repetitions; i++) {
        times.push_back(measure_time(function));
    }
    std::sort(times.begin(), times.end());
    return times[times.size() / 2];
}

void benchmark_raw(int count) {
    std::vector<int*> values;
    values.reserve(count);
    for (int i = 0; i < count; i++) {
        values.push_back(new int(i));
    }
    for (int* value : values) {
        delete value;
    }
}

void benchmark_std_unique(int count) {
    std::vector<std::unique_ptr<int>> values;
    values.reserve(count);
    for (int i = 0; i < count; i++) {
        values.push_back(std::make_unique<int>(i));
    }
}

void benchmark_unq(int count) {
    std::vector<UnqPtr<int>> values;
    values.reserve(count);
    for (int i = 0; i < count; i++) {
        values.emplace_back(new int(i));
    }
}

void benchmark_std_shared(int count) {
    std::vector<std::shared_ptr<int>> values;
    values.reserve(count);
    for (int i = 0; i < count; i++) {
        values.push_back(std::make_shared<int>(i));
    }
}

void benchmark_shrd(int count) {
    std::vector<ShrdPtr<int>> values;
    values.reserve(count);
    for (int i = 0; i < count; i++) {
        values.emplace_back(new int(i));
    }
}

template <class Pointer>
double benchmark_copy(int count, int repetitions) {
    Pointer original(new int(42));
    return measure_median([&]() {
        std::vector<Pointer> copies;
        copies.reserve(count);
        for (int i = 0; i < count; i++) {
            copies.push_back(original);
        }
    }, repetitions);
}

template <class Pointer>
double benchmark_move(int count, int repetitions) {
    std::vector<Pointer> source;
    source.reserve(count);
    for (int i = 0; i < count; i++) {
        source.emplace_back(new int(i));
    }
    return measure_median([&]() {
        std::vector<Pointer> destination;
        destination.reserve(count);
        for (int i = 0; i < count; i++) {
            destination.push_back(std::move(source[i]));
        }
        for (int i = 0; i < count; i++) {
            source[i] = std::move(destination[i]);
        }
    }, repetitions);
}

int main() {
    const std::vector<int> sizes = {1000, 10000, 100000, 500000};
    const int repetitions = 5;
    std::ofstream file("benchmark_results.csv");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create benchmark_results.csv");
    }
    file << "count,raw,std_unique,unq,std_shared,shrd\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Benchmark: creation and destruction\n";
    std::cout << "Time in milliseconds, median of " << repetitions << " runs\n\n";
    for (int count : sizes) {
        double raw = measure_median([&]() {
            benchmark_raw(count);
        }, repetitions);
        double std_unique = measure_median([&]() {
            benchmark_std_unique(count);
        }, repetitions);
        double unq = measure_median([&]() {
            benchmark_unq(count);
        }, repetitions);
        double std_shared = measure_median([&]() {
            benchmark_std_shared(count);
        }, repetitions);
        double shrd = measure_median([&]() {
            benchmark_shrd(count);
        }, repetitions);
        std::cout << "N = " << count << "\n";
        std::cout << "  raw:        " << raw << " ms\n";
        std::cout << "  std_unique: " << std_unique << " ms\n";
        std::cout << "  unq:        " << unq << " ms\n";
        std::cout << "  std_shared: " << std_shared << " ms\n";
        std::cout << "  shrd:       " << shrd << " ms\n\n";
        file << count << ","
             << raw << ","
             << std_unique << ","
             << unq << ","
             << std_shared << ","
             << shrd << "\n";
    }
    std::cout << "Results saved to benchmark_results.csv\n";

    std::ofstream copy_file("copy_results.csv");
    if (!copy_file.is_open()) {
        throw std::runtime_error("Cannot create copy_results.csv");
    }
    copy_file << "count,shrd_copy,std_shared_copy,unq_move,shrd_move,std_unique_move\n";
    std::cout << "\nBenchmark: copying and moving\n";
    std::cout << "Time in milliseconds, median of " << repetitions << " runs\n\n";
    for (int count : sizes) {
        double shrd_copy = benchmark_copy<ShrdPtr<int>>(count, repetitions);
        double std_shared_copy = benchmark_copy<std::shared_ptr<int>>(count, repetitions);
        double unq_move = benchmark_move<UnqPtr<int>>(count, repetitions);
        double shrd_move = benchmark_move<ShrdPtr<int>>(count, repetitions);
        double std_unique_move = benchmark_move<std::unique_ptr<int>>(count, repetitions);
        std::cout << "N = " << count << "\n";
        std::cout << "  ShrdPtr copy:         " << shrd_copy << " ms\n";
        std::cout << "  std::shared_ptr copy: " << std_shared_copy << " ms\n";
        std::cout << "  UnqPtr move:          " << unq_move << " ms\n";
        std::cout << "  ShrdPtr move:         " << shrd_move << " ms\n";
        std::cout << "  std::unique_ptr move: " << std_unique_move << " ms\n\n";
        copy_file << count << ","
                  << shrd_copy << ","
                  << std_shared_copy << ","
                  << unq_move << ","
                  << shrd_move << ","
                  << std_unique_move << "\n";
    }
    std::cout << "Results saved to copy_results.csv\n";
    return 0;
}
