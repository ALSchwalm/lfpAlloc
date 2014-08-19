
#include <lfpAlloc/PoolDispatcher.hpp>
#include <gtest/gtest.h>

TEST(DispatcherTest, Allocate) {
    lfpAlloc::PoolDispatcher<int> dispatcher;

    std::vector<int*> v;
    for (std::size_t s=0; s < 5e5; ++s) {
        EXPECT_NE(dispatcher.allocate(1), nullptr);
    }
}

TEST(DispatcherTest, Deallocate) {
    lfpAlloc::PoolDispatcher<int> dispatcher;

    std::vector<int*> v;
    for (std::size_t s=0; s < 5e5; ++s) {
        auto p = dispatcher.allocate(1);
        dispatcher.deallocate(p, 1);
    }
}
