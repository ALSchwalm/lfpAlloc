
#include <lfpAlloc/Pool.hpp>
#include <gtest/gtest.h>

TEST(PoolTest, Allocate) {
    lfpAlloc::Pool<int, 1, 40000> pool;

    std::vector<int*> v;
    for (std::size_t s=0; s < 5e6; ++s) {
        EXPECT_NE(pool.allocate(), nullptr);
    }
}
