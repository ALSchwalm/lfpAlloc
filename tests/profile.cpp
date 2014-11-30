
#include <lfpAlloc/Allocator.hpp>
#include <list>
#include <algorithm>
#include <chrono>
#include <iostream>

using lfpAlloc::lfpAllocator;

namespace utils {
template <typename Func>
void multiRun(Func fun, int thread_count) {
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(fun);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

template <typename T>
std::chrono::milliseconds asMS(const T& time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(time);
}

template <typename Func, typename... Ts>
std::chrono::milliseconds timeCall(Func func, Ts&&... args) {
    auto start = std::chrono::system_clock::now();
    func(args...);
    auto end = std::chrono::system_clock::now();
    return asMS(end - start);
}

std::size_t random_uint(std::size_t lower, std::size_t upper) {
    static std::default_random_engine e{};
    std::uniform_int_distribution<std::size_t> d{lower, upper - 1};
    return d(e);
}
} // End utils

template <typename T>
void addRemovePredicatableBody(T& list, int size = 4e5) {
    for (int s = 0; s < size; ++s) {
        list.push_back(s);
    }

    int count = 0;
    for (auto iter = list.begin(); iter != list.end(); ++iter, ++count) {
        if (count % 3 == 0) {
            iter = list.erase(iter);
        }
    }
}

template <typename T>
void addRemoveRandomBody(T& list, int size = 4e3) {
    for (int s = 0; s < size; ++s) {
        list.push_back(s);
    }

    while (list.size() > static_cast<std::size_t>(size / 2)) {
        auto pos = list.begin();
        std::advance(pos, utils::random_uint(0, list.size()));
        list.erase(pos);
    }
}

void lfpAddRemovePredicatable(int thread_count) {
    std::vector<std::thread> threads;
    lfpAllocator<int> alloc;
    auto fun = [&alloc] {
        std::list<int, lfpAllocator<int>> list(alloc);
        addRemovePredicatableBody(list);
    };
    utils::multiRun(fun, thread_count);
}

void stdAddRemovePredictable(int thread_count) {
    auto fun = [] {
        std::list<int> list;
        addRemovePredicatableBody(list);
    };
    utils::multiRun(fun, thread_count);
}

void lfpAddRemoveRandom(int thread_count) {
    std::vector<std::thread> threads;
    lfpAllocator<int> alloc;
    auto fun = [&alloc] {
        std::list<int, lfpAllocator<int>> list(alloc);
        addRemoveRandomBody(list);
    };
    utils::multiRun(fun, thread_count);
}

void stdAddRemoveRandom(int thread_count) {
    auto fun = [] {
        std::list<int> list;
        addRemoveRandomBody(list);
    };
    utils::multiRun(fun, thread_count);
}

int main() {
    std::chrono::milliseconds time;

    std::cout << "Profile - Add/Remove Random" << std::endl;
    for (int i = 1; i <= 32; i <<= 1) {
        std::cout << "  Using " << i << " threads " << std::endl;

        time = utils::timeCall(lfpAddRemoveRandom, i);
        std::cout << "   LFP Allocator time (ms): " << time.count()
                  << std::endl;

        time = utils::timeCall(stdAddRemoveRandom, i);
        std::cout << "   std Allocator time (ms): " << time.count()
                  << std::endl;
    }

    std::cout << "Profile - Add/Remove Predictable" << std::endl;
    for (int i = 1; i <= 32; i <<= 1) {
        std::cout << "  Using " << i << " threads " << std::endl;

        time = utils::timeCall(lfpAddRemovePredicatable, i);
        std::cout << "   LFP Allocator time (ms): " << time.count()
                  << std::endl;

        time = utils::timeCall(stdAddRemovePredictable, i);
        std::cout << "   std Allocator time (ms): " << time.count()
                  << std::endl;
    }
}
