
#include <lfpAlloc/Allocator.hpp>
#include <list>
#include <algorithm>
#include <chrono>
#include <iostream>

template<typename T>
void addRemoveTestBody(T& list) {
    for (std::size_t s=0; s<100e5; ++s) {
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
    auto fun = [&alloc]{
        std::list<int, decltype(alloc)> list(alloc);
        addRemoveTestBody(list);
    };
    auto t1 = std::thread(fun);
    auto t2 = std::thread(fun);
    auto t3 = std::thread(fun);
    auto t4 = std::thread(fun);

    t1.join(); t2.join(); t3.join(); t4.join();
}

void stdAddRemoveMultithreadTest() {
    auto fun = []{
        std::list<int> list;
        addRemoveTestBody(list);
    };
    auto t1 = std::thread(fun);
    auto t2 = std::thread(fun);
    auto t3 = std::thread(fun);
    auto t4 = std::thread(fun);

    t1.join(); t2.join(); t3.join(); t4.join();
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
