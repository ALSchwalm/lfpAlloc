
#include <lfpAlloc/Allocator.hpp>
#include <list>
#include <algorithm>
#include <chrono>
#include <iostream>

template<typename T>
void addRemoveTestBody(T& list) {
    for (std::size_t s=0; s<4e5; ++s) {
        list.push_back(s);
    }

    int count=0;
    for (auto iter = list.begin(); iter != list.end(); ++iter,++count) {
        if (count%3==0) {
            iter = list.erase(iter);
        }
    }
}

void lfpAddRemoveMultithreadTest() {
    lfpAlloc::lfpAllocator<int> alloc;
    std::vector<std::thread> threads;
    auto fun = [&alloc]{
        std::list<int, decltype(alloc)> list(alloc);
        addRemoveTestBody(list);
    };
    for (int i=0; i < 16; ++i) {
        threads.emplace_back(fun);
    }

    for (auto& thread : threads){
        thread.join();
    }
}

void stdAddRemoveMultithreadTest() {
    std::vector<std::thread> threads;
    auto fun = []{
        std::list<int> list;
        addRemoveTestBody(list);
    };
    for (int i=0; i < 16; ++i) {
        threads.emplace_back(fun);
    }

    for (auto& thread : threads){
        thread.join();
    }
}

void lfpAddRemoveTest() {
    std::list<int, lfpAlloc::lfpAllocator<int>> list;
    addRemoveTestBody(list);
}

void stdAddRemoveTest() {
    std::list<int> list;
    addRemoveTestBody(list);
}

template<typename T>
std::chrono::milliseconds asMS(const T& time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(time);
}

int main() {
    std::cout << "Profile - Add/Remove Multiple Thread" << std::endl;
    auto start = std::chrono::system_clock::now();
    lfpAddRemoveMultithreadTest();
    auto end = std::chrono::system_clock::now();
    auto elapsed = end - start;
    std::cout << "  LFP Allocator time (ms): " << asMS(elapsed).count() << std::endl;

    start = std::chrono::system_clock::now();
    stdAddRemoveMultithreadTest();
    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "  std Allocator time (ms): " << asMS(elapsed).count() << std::endl;


    std::cout << "Profile - Add/Remove Single Thread" << std::endl;
    start = std::chrono::system_clock::now();
    lfpAddRemoveTest();
    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "  LFP Allocator time (ms): " << asMS(elapsed).count() << std::endl;

    start = std::chrono::system_clock::now();
    stdAddRemoveTest();
    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "  std Allocator time (ms): " << asMS(elapsed).count() << std::endl;
}
