
#include <lfpAlloc/PoolDispatcher.hpp>
#include <gtest/gtest.h>

TEST(DispatcherTest, Allocate) {
    lfpAlloc::PoolDispatcher<int, 8> dispatcher;

    std::vector<int*> v;
    for (std::size_t s=0; s < 5e6; ++s) {
        EXPECT_NE(dispatcher.allocate(1), nullptr);
    }
}

TEST(DispatcherTest, Deallocate) {
    lfpAlloc::PoolDispatcher<int, 8> dispatcher;

    std::vector<int*> v;
    for (std::size_t s=0; s < 5e6; ++s) {
        auto p = dispatcher.allocate(1);
        v.push_back(p);
    }

    for (auto p : v) {
        dispatcher.deallocate(p, 1);
    }
}
