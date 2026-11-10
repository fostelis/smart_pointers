#include <type_traits>
#include <windows.h>
#include <psapi.h>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include "unq_ptr.h"
#include "shrd_ptr.h"

size_t get_memory_usage() {
    PROCESS_MEMORY_COUNTERS counters{};
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
        throw std::runtime_error("Cannot measure process memory");
    }
    return counters.WorkingSetSize;
}

double bytes_to_mb(size_t bytes) {
    return static_cast<double>(bytes) / (1024.0 * 1024.0);
}

template <class Pointer, class Factory>
double measure_memory(int count, Factory factory) {
    std::vector<Pointer> values;
    values.reserve(count);
    size_t before = get_memory_usage();
    for (int i = 0; i < count; i++) {
        values.push_back(factory(i));
    }
    size_t after = get_memory_usage();
    if constexpr (std::is_same_v<Pointer, int*>) {
        for (int* value : values) {
            delete value;
        }
    }
    return static_cast<double>(after) - static_cast<double>(before);
}

int main() {
    const std::vector<int> sizes = {1000, 10000, 100000, 500000};
    std::ofstream file("memory_results.csv");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create memory_results.csv");
    }
    file << "count,raw,std_unique,unq,std_shared,shrd\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Benchmark: memory usage\n";
    std::cout << "Memory in MB (Working Set delta)\n\n";
    for (int count : sizes) {
        double raw = measure_memory<int*>(count, [](int i) {
            return new int(i);
        });
        double std_unique = measure_memory<std::unique_ptr<int>>(count, [](int i) {
            return std::make_unique<int>(i);
        });
        double unq = measure_memory<UnqPtr<int>>(count, [](int i) {
            return UnqPtr<int>(new int(i));
        });
        double std_shared = measure_memory<std::shared_ptr<int>>(count, [](int i) {
            return std::make_shared<int>(i);
        });
        double shrd = measure_memory<ShrdPtr<int>>(count, [](int i) {
            return ShrdPtr<int>(new int(i));
        });
        std::cout << "N = " << count << "\n";
        std::cout << "  raw:        " << bytes_to_mb(raw) << " MB\n";
        std::cout << "  std_unique: " << bytes_to_mb(std_unique) << " MB\n";
        std::cout << "  unq:        " << bytes_to_mb(unq) << " MB\n";
        std::cout << "  std_shared: " << bytes_to_mb(std_shared) << " MB\n";
        std::cout << "  shrd:       " << bytes_to_mb(shrd) << " MB\n\n";
        file << count << ","
             << bytes_to_mb(raw) << ","
             << bytes_to_mb(std_unique) << ","
             << bytes_to_mb(unq) << ","
             << bytes_to_mb(std_shared) << ","
             << bytes_to_mb(shrd) << "\n";
    }
    std::cout << "Results saved to memory_results.csv\n";
    return 0;
}
