#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "../include/lockfree_stack.hpp"
#include "../include/mutex_stack.hpp"

constexpr int NUM_THREADS = 8;
constexpr int OPERATIONS = 100000;

template<typename Stack>
void benchmark_stack(const std::string& name) {
    Stack stack;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < OPERATIONS; ++j) {
                stack.push(j);
                int value;
                stack.pop(value);
            }
        });
    }

    for (auto& t : threads)
        t.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << name << " took " << duration.count() << " ms\n";
}

int main() {
    benchmark_stack<MutexStack<int>>("Mutex Stack");
    benchmark_stack<LockFreeStack<int>>("Lock-Free Stack");

    return 0;
}
