
cmake_minimum_required(VERSION 3.20)

project(smart_pointers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(smart_pointers
        main.cpp
)

target_include_directories(smart_pointers PRIVATE include)

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(smart_pointers_tests
        tests/unq_ptr_tests.cpp
        tests/shrd_ptr_tests.cpp
        tests/inheritance_tests.cpp
        tests/safety_tests.cpp
        tests/container_tests.cpp
        tests/benchmark_tests.cpp
)

target_include_directories(smart_pointers_tests PRIVATE include)

target_link_libraries(
        smart_pointers_tests
        PRIVATE
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(smart_pointers_tests)

add_executable(smart_pointers_benchmark
        benchmarks/benchmark.cpp
)
target_include_directories(smart_pointers_benchmark PRIVATE include)

add_executable(smart_pointers_memory
        benchmarks/memory_benchmark.cpp
)
target_include_directories(smart_pointers_memory PRIVATE include)
target_link_libraries(smart_pointers_memory PRIVATE psapi)


add_executable(smart_pointers_dashboard
        dashboard/dashboard.cpp
)
target_compile_definitions(smart_pointers_dashboard PRIVATE NOMINMAX)
target_link_libraries(smart_pointers_dashboard PRIVATE shell32)
add_dependencies(smart_pointers_dashboard
        smart_pointers_tests
        smart_pointers_benchmark
        smart_pointers_memory
)
